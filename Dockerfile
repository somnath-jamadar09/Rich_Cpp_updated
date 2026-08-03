# Stage 1: Build binary using GCC C++17 compiler
FROM gcc:12 AS builder
WORKDIR /app
COPY . .
RUN g++ -std=c++17 -Wall -Wextra -Isrc src/main.cpp -o rich_demo

# Stage 2: Runtime image containing compiled artifact and tests
FROM python:3.11-slim
WORKDIR /app

# Install pytest for running test suites if desired inside container
RUN pip install --no-cache-dir pytest

COPY --from=builder /app/rich_demo /app/rich_demo
COPY --from=builder /app/src /app/src
COPY --from=builder /app/tests /app/tests
COPY --from=builder /app/LICENSE /app/LICENSE
COPY --from=builder /app/README.md /app/README.md

ENV PYTHONIOENCODING=utf-8
CMD ["/app/rich_demo"]
