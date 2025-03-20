import {
	GenericContainer,
	type StartedTestContainer,
	Wait,
} from "testcontainers";
import { fileURLToPath } from "node:url";
import test from "node:test";
import assert from "node:assert";
import postgres from "postgres";

const PORT = 5432;

let startedContainer: StartedTestContainer;
const env = {
	POSTGRES_USER: "postgres",
	POSTGRES_PASSWORD: "postgres",
};

test.before(async () => {
	const container = await GenericContainer.fromDockerfile(
		fileURLToPath(new URL(".", import.meta.url)),
	).build();

	startedContainer = await container
		.withHealthCheck({
			test: [
				"CMD-SHELL",
				`pg_isready -U ${env.POSTGRES_USER} --dbname=postgres`,
			],
			interval: 1000,
			timeout: 5000,
			retries: 5,
			startPeriod: 1000,
		})
		.withWaitStrategy(Wait.forHealthCheck())
		.withEnvironment(env)
		.withExposedPorts(PORT)
		.withCommand("postgres -c wal_compression=zstd".split(" "))
		.start();
});

test("implicit schema creation", async (t) => {
	const host = startedContainer.getHost();
	const port = startedContainer.getMappedPort(PORT);
	const connectionString = `postgres://postgres:postgres@${host}:${port}/postgres`;
	const sql = postgres(connectionString);

	await sql`CREATE TABLE implicit_schema.table1 (id SERIAL PRIMARY KEY)`;

	const schemas = (
		await sql`SELECT schema_name FROM information_schema.schemata`
	).filter((schema) => schema.schema_name === "implicit_schema");

	assert(schemas.length === 1, "schema was not created");

	await sql.end({ timeout: 5 });
});

test.after(async () => {
	await startedContainer.stop({ remove: true, removeVolumes: true });
});
