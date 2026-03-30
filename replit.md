# CEP 146 - NDD Lab 12: Apartment Rent Splitter

## Project Overview
A C program that fairly splits monthly rent and utilities among 4 apartment
tenants using weighted splitting (room size) with persistent roommate profiles
and an accumulating ledger across months.

## Features
- Weighted rent split by room square footage (larger room = larger share)
- Equal utilities split among all tenants
- Saves roommate names + room sizes to `roommates.dat` (loaded on next run)
- Accumulates per-person totals in `ledger.dat` across months
- Saves a full receipt to `rent_breakdown.txt` after every run
- Polished ASCII table output with current month breakdown + running totals

## Files
| File               | Purpose                                      |
|--------------------|----------------------------------------------|
| `main.c`           | Full application source (C99, single file)   |
| `Makefile`         | Build config using gcc                       |
| `roommates.dat`    | Saved tenant names and room sizes            |
| `ledger.dat`       | Accumulated rent/util totals per tenant      |
| `rent_breakdown.txt` | Receipt from the most recent run           |
| `lab12`            | Compiled binary (gitignored)                 |

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
No external libraries — pure C99, works on any standard Linux system.

## Weighted Split Formula
```
rent_share_i  = (room_sqft_i / total_sqft) * total_rent
util_share_i  = total_utilities / 4          (equal split)
total_owed_i  = rent_share_i + util_share_i
```

## Workflow
- **Start application**: Runs `make && ./lab12` (console output)
