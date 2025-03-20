# PG Juice

[JuiceFS](https://juicefs.com) is a distributed file system that can use
PostgreSQL as its metadata storage engine. While JuiceFS can technically create
multiple filesystems in a single PostgreSQL database, it requires the user to
manually (or outside of JuiceFS) create a new schema for each filesystem. This
can be especially cumbersome when
[managing JuiceFS mount points through Docker](https://juicefs.com/docs/community/juicefs_on_docker).

PG Juice is an extended version of PostgreSQL that simplifies file system
creation by automatically creating schemas for each filesystem.

In other words, **PG Juice** allows the following:

```bash
PG_URL="postgres://username:password@pg-juice:5432/juicefs"

# Create the first filesystem
juicefs format \
    # ... other options ...
    "${PG_URL}?search_path=fs1" fs1

# Create the second filesystem (no need to create a new schema explicitly)
juicefs format \
    # ... other options ...
    "${PG_URL}?search_path=fs2" fs2
```

## Quick Start

docker-compose.yml:

```yaml
services:
    pg-juice:
        image: fardjad/pg-juice:17 # 17 is the version of PostgreSQL
        restart: unless-stopped
        healthcheck:
            test: "pg_isready -U ${POSTGRES_USER:-juicefs} --dbname=juicefs"
            interval: 10s
            timeout: 5s
            retries: 5
        environment:
            POSTGRES_DB: juicefs
            POSTGRES_USER: "${POSTGRES_USER:-juicefs}"
            POSTGRES_PASSWORD: "${POSTGRES_PASSWORD:-changeme}"
        command: postgres -c wal_compression=zstd
        ports:
            - "5432:5432"
        volumes:
            - ./pg-data:/var/lib/postgresql/data
```

## Running the Tests

To run the integration tests, you need to have Docker and the latest LTS version
of Node installed. Then run:

```bash
npm install
npm test
```
