# CEP 146 - NDD Lab 12: Apartment Rent Splitter

## Project Overview
A C program that fairly splits monthly rent among 4 apartment tenants using
weighted splitting based on each room's square footage. Larger rooms pay more.

## Features
- Enter total monthly rent amount
- Enter each tenant's name and room size (sq ft)
- Calculates each person's weighted share (room_sqft / total_sqft)
- Displays a formatted breakdown table and per-person summary
- Rounding-safe: final tenant absorbs any floating-point remainder

## Structure
- `main.c`   — Full application source (C99, single file)
- `Makefile`  — Build configuration using gcc
- `lab12`     — Compiled binary (gitignored)

## Build & Run
```bash
make        # Compile → produces ./lab12
./lab12     # Run interactively
make clean  # Remove compiled binary
```

## Compiling on Matrix (university Linux server)
```bash
gcc -Wall -Wextra -pedantic -std=c99 -g -o lab12 main.c
./lab12
```

## Workflow
- **Start application**: Runs `make && ./lab12` (console output)

## Weighted Split Formula
```
share_i       = room_sqft_i / sum(all room_sqft)
amount_owed_i = share_i * total_rent
```
