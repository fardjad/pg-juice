FROM postgres:17 AS builder

RUN apt update \
    && apt install -y \
    build-essential \
    postgresql-server-dev-17

COPY implicit_schema/Makefile implicit_schema/implicit_schema.c implicit_schema/implicit_schema.control ./
RUN make

FROM postgres:17

COPY --from=builder /implicit_schema.so /usr/lib/postgresql/17/lib/implicit_schema.so
COPY implicit_schema/implicit_schema.control implicit_schema/implicit_schema--1.0.sql /usr/share/postgresql/17/extension/

COPY implicit_schema/init_implicit_schema.sql /docker-entrypoint-initdb.d/implicit_schema.sql
