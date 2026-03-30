#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define NUM_ROOMS    4
#define NAME_LEN     24
#define PROFILE_FILE "roommates.dat"
#define LEDGER_FILE  "ledger.dat"
#define RECEIPT_FILE "rent_breakdown.txt"

/* ------------------------------------------------------------------ */
/*  Data                                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    char   name[NAME_LEN];
    double sqft;
    double share;
    /* this month */
    double rent_now;
    double util_now;
    double total_now;
    /* running ledger */
    double cum_rent;
    double cum_util;
    double cum_total;
    int    months;
} Tenant;

/* ------------------------------------------------------------------ */
/*  Input helpers                                                       */
/* ------------------------------------------------------------------ */

static void flush(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static double prompt_positive(const char *msg)
{
    double v;
    printf("%s", msg);
    while (scanf("%lf", &v) != 1 || v < 0.0) {
        printf("  Please enter 0 or a positive number: ");
        flush();
    }
    flush();
    return v;
}

static char prompt_yn(const char *msg)
{
    char buf[8];
    printf("%s", msg);
    while (1) {
        if (fgets(buf, sizeof(buf), stdin)) {
            char c = (char)tolower((unsigned char)buf[0]);
            if (c == 'y' || c == 'n') return c;
        }
        printf("  Enter y or n: ");
    }
}

/* ------------------------------------------------------------------ */
/*  Profile (roommate config)                                           */
/* ------------------------------------------------------------------ */

static int load_profile(Tenant t[], int n)
{
    FILE *f = fopen(PROFILE_FILE, "r");
    if (!f) return 0;
    int i, ok = 1;
    for (i = 0; i < n && ok; i++) {
        if (fscanf(f, " %23[^|]|%lf\n", t[i].name, &t[i].sqft) != 2)
            ok = 0;
    }
    fclose(f);
    return ok;
}

static void save_profile(Tenant t[], int n)
{
    FILE *f = fopen(PROFILE_FILE, "w");
    if (!f) { printf("  Warning: could not save roommate profile.\n"); return; }
    int i;
    for (i = 0; i < n; i++)
        fprintf(f, "%s|%.2f\n", t[i].name, t[i].sqft);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/*  Ledger (running totals)                                             */
/* ------------------------------------------------------------------ */

static void init_ledger(Tenant t[], int n)
{
    int i;
    for (i = 0; i < n; i++) {
        t[i].cum_rent = t[i].cum_util = t[i].cum_total = 0.0;
        t[i].months = 0;
    }
}

static int load_ledger(Tenant t[], int n)
{
    FILE *f = fopen(LEDGER_FILE, "r");
    if (!f) { init_ledger(t, n); return 0; }
    char name[NAME_LEN];
    int i, ok = 1;
    for (i = 0; i < n && ok; i++) {
        if (fscanf(f, " %23[^|]|%lf|%lf|%d\n",
                   name,
                   &t[i].cum_rent,
                   &t[i].cum_util,
                   &t[i].months) != 4)
            ok = 0;
    }
    fclose(f);
    if (!ok) init_ledger(t, n);
    return ok;
}

static void save_ledger(Tenant t[], int n)
{
    FILE *f = fopen(LEDGER_FILE, "w");
    if (!f) { printf("  Warning: could not save ledger.\n"); return; }
    int i;
    for (i = 0; i < n; i++)
        fprintf(f, "%s|%.2f|%.2f|%d\n",
                t[i].name, t[i].cum_rent, t[i].cum_util, t[i].months);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/*  Pretty-print helpers                                                */
/* ------------------------------------------------------------------ */

#define W 74   /* total line width */

static void rule_double(FILE *fp)
{
    int i;
    fputc('+', fp);
    for (i = 0; i < W - 2; i++) fputc('=', fp);
    fputc('+', fp);
    fputc('\n', fp);
}

static void rule_single(FILE *fp)
{
    int i;
    fputc('+', fp);
    for (i = 0; i < W - 2; i++) fputc('-', fp);
    fputc('+', fp);
    fputc('\n', fp);
}

static void banner(FILE *fp, const char *text)
{
    /* centre text inside W chars */
    int pad  = (W - 2 - (int)strlen(text)) / 2;
    int rpad = (W - 2) - pad - (int)strlen(text);
    fprintf(fp, "|%*s%s%*s|\n", pad, "", text, rpad, "");
}

static void kv(FILE *fp, const char *key, const char *val)
{
    char buf[W + 4];
    snprintf(buf, sizeof(buf), "  %-14s %s", key, val);
    int rpad = (W - 2) - (int)strlen(buf);
    if (rpad < 0) rpad = 0;
    fprintf(fp, "|%s%*s|\n", buf, rpad, "");
}

/* Monthly breakdown table row */
static void row_month(FILE *fp,
                       const char *name, double sqft, double share,
                       double rent, double util, double total)
{
    fprintf(fp, "| %-16s | %7.1f | %5.1f%% | %9.2f | %9.2f | %9.2f |\n",
            name, sqft, share * 100.0, rent, util, total);
}

/* Ledger table row */
static void row_ledger(FILE *fp,
                        const char *name,
                        double cum_rent, double cum_util,
                        double cum_total, int months)
{
    fprintf(fp, "| %-16s | %10.2f | %10.2f | %10.2f | %6d |\n",
            name, cum_rent, cum_util, cum_total, months);
}

static void table_header_month(FILE *fp)
{
    fprintf(fp, "| %-16s | %7s | %6s | %9s | %9s | %9s |\n",
            "Tenant", "Sq Ft", "Share", "Rent ($)", "Utils ($)", "Total ($)");
}

static void table_header_ledger(FILE *fp)
{
    fprintf(fp, "| %-16s | %10s | %10s | %10s | %6s |\n",
            "Tenant", "Rent ($)", "Utils ($)", "Total ($)", "Months");
}

/* ------------------------------------------------------------------ */
/*  Full report (prints to any FILE*)                                   */
/* ------------------------------------------------------------------ */

static void print_report(FILE *fp, Tenant t[], int n,
                          double total_sqft,
                          const char *ts, const char *month_str)
{
    int i;

    /* === HEADER === */
    rule_double(fp);
    banner(fp, "APARTMENT RENT SPLITTER");
    banner(fp, "Weighted by Room Size");
    rule_double(fp);
    kv(fp, "Generated:", ts);
    kv(fp, "Month:", month_str);
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d months on record", t[0].months);
        kv(fp, "History:", buf);
    }
    kv(fp, "Apartment:", "4 rooms — see breakdown below");

    /* === THIS MONTH === */
    rule_double(fp);
    banner(fp, "THIS MONTH'S BREAKDOWN");
    rule_single(fp);
    table_header_month(fp);
    rule_single(fp);

    for (i = 0; i < n; i++)
        row_month(fp, t[i].name, t[i].sqft, t[i].share,
                  t[i].rent_now, t[i].util_now, t[i].total_now);

    rule_single(fp);

    /* totals row */
    {
        double tr = 0, tu = 0, tt = 0;
        for (i = 0; i < n; i++) { tr += t[i].rent_now; tu += t[i].util_now; tt += t[i].total_now; }
        row_month(fp, "TOTAL", total_sqft, 1.0, tr, tu, tt);
    }
    rule_double(fp);

    /* per-person summary */
    fprintf(fp, "|\n");
    banner(fp, "What each person owes this month:");
    fprintf(fp, "|\n");
    for (i = 0; i < n; i++) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "  %-16s  rent $%8.2f  +  utils $%7.2f  =  TOTAL $%8.2f",
                 t[i].name,
                 t[i].rent_now, t[i].util_now, t[i].total_now);
        int rpad = (W - 2) - (int)strlen(buf);
        if (rpad < 0) rpad = 0;
        fprintf(fp, "|%s%*s|\n", buf, rpad, "");
    }
    fprintf(fp, "|\n");

    /* === RUNNING LEDGER === */
    rule_double(fp);
    banner(fp, "RUNNING TOTALS  (all months combined)");
    rule_single(fp);
    table_header_ledger(fp);
    rule_single(fp);

    for (i = 0; i < n; i++)
        row_ledger(fp, t[i].name,
                   t[i].cum_rent, t[i].cum_util, t[i].cum_total,
                   t[i].months);

    rule_single(fp);
    {
        double tr = 0, tu = 0, tt = 0;
        for (i = 0; i < n; i++) { tr += t[i].cum_rent; tu += t[i].cum_util; tt += t[i].cum_total; }
        row_ledger(fp, "TOTAL", tr, tu, tt, t[0].months);
    }
    rule_double(fp);

    fprintf(fp, "|\n");
    banner(fp, "Outstanding balance per person  (all time):");
    fprintf(fp, "|\n");
    for (i = 0; i < n; i++) {
        char buf[W + 4];
        snprintf(buf, sizeof(buf),
                 "  %-16s  $%.2f total owed over %d month%s",
                 t[i].name, t[i].cum_total,
                 t[i].months, t[i].months == 1 ? "" : "s");
        int rpad = (W - 2) - (int)strlen(buf);
        if (rpad < 0) rpad = 0;
        fprintf(fp, "|%s%*s|\n", buf, rpad, "");
    }
    fprintf(fp, "|\n");
    rule_double(fp);
}

/* ------------------------------------------------------------------ */
/*  Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    Tenant tenants[NUM_ROOMS];
    double total_rent = 0.0;
    double total_util = 0.0;
    double total_sqft = 0.0;
    int    i;

    /* zero everything so no field is ever uninitialized */
    memset(tenants, 0, sizeof(tenants));

    /* timestamp */
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char   ts[32], month_str[32];
    strftime(ts,         sizeof(ts),         "%Y-%m-%d %H:%M:%S", tm_now);
    strftime(month_str,  sizeof(month_str),  "%B %Y",             tm_now);

    /* ---- welcome ---- */
    rule_double(stdout);
    banner(stdout, "APARTMENT RENT SPLITTER");
    banner(stdout, "Weighted by Room Size  |  Persistent Ledger");
    rule_double(stdout);
    printf("\n");

    /* ---- load or enter roommate profile ---- */
    int have_profile = load_profile(tenants, NUM_ROOMS);

    if (have_profile) {
        printf("  Saved roommates found:\n\n");
        for (i = 0; i < NUM_ROOMS; i++)
            printf("    Room %d: %-20s  (%.0f sq ft)\n",
                   i + 1, tenants[i].name, tenants[i].sqft);
        printf("\n");
        char choice = prompt_yn("  Use these roommates? (y/n): ");
        printf("\n");
        if (choice == 'n') have_profile = 0;
    }

    if (!have_profile) {
        printf("  Enter the name and room size for each of the %d rooms.\n\n",
               NUM_ROOMS);
        for (i = 0; i < NUM_ROOMS; i++) {
            printf("  Room %d\n", i + 1);
            printf("    Tenant name       : ");
            fgets(tenants[i].name, NAME_LEN, stdin);
            tenants[i].name[strcspn(tenants[i].name, "\n")] = '\0';
            if (tenants[i].name[0] == '\0')
                snprintf(tenants[i].name, NAME_LEN, "Tenant %d", i + 1);

            char sqft_msg[64];
            snprintf(sqft_msg, sizeof(sqft_msg),
                     "    Room size (sq ft) : ");
            tenants[i].sqft = prompt_positive(sqft_msg);
            printf("\n");
        }
        save_profile(tenants, NUM_ROOMS);
        printf("  Roommate profile saved.\n\n");
    }

    /* ---- load running ledger ---- */
    load_ledger(tenants, NUM_ROOMS);
    /* derive cum_total from stored fields so it is always consistent */
    for (i = 0; i < NUM_ROOMS; i++)
        tenants[i].cum_total = tenants[i].cum_rent + tenants[i].cum_util;

    /* ---- total sqft ---- */
    total_sqft = 0.0;
    for (i = 0; i < NUM_ROOMS; i++)
        total_sqft += tenants[i].sqft;

    /* ---- this month's amounts ---- */
    rule_single(stdout);
    printf("\n");
    total_rent = prompt_positive("  Total monthly RENT      ($): ");
    total_util = prompt_positive("  Total monthly UTILITIES  ($): ");
    printf("\n");

    /* ---- weighted calculation ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        tenants[i].share    = tenants[i].sqft / total_sqft;
        tenants[i].rent_now = tenants[i].share * total_rent;
        tenants[i].util_now = total_util / NUM_ROOMS;
    }

    /* rounding correction on last tenant — applied BEFORE any output */
    {
        double run_r = 0.0, run_u = 0.0;
        for (i = 0; i < NUM_ROOMS - 1; i++) {
            run_r += tenants[i].rent_now;
            run_u += tenants[i].util_now;
        }
        tenants[NUM_ROOMS - 1].rent_now = total_rent - run_r;
        tenants[NUM_ROOMS - 1].util_now = total_util - run_u;
    }

    /* combined totals */
    for (i = 0; i < NUM_ROOMS; i++)
        tenants[i].total_now = tenants[i].rent_now + tenants[i].util_now;

    /* ---- update ledger ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        tenants[i].cum_rent  += tenants[i].rent_now;
        tenants[i].cum_util  += tenants[i].util_now;
        tenants[i].cum_total += tenants[i].total_now;
        tenants[i].months    += 1;
    }
    save_ledger(tenants, NUM_ROOMS);

    /* ---- print to screen ---- */
    printf("\n");
    print_report(stdout, tenants, NUM_ROOMS,
                 total_sqft, ts, month_str);

    /* ---- save receipt ---- */
    FILE *fp = fopen(RECEIPT_FILE, "w");
    if (fp) {
        print_report(fp, tenants, NUM_ROOMS,
                     total_sqft, ts, month_str);
        fclose(fp);
        printf("\n  Receipt saved to: %s\n", RECEIPT_FILE);
    } else {
        printf("\n  Warning: could not write receipt file.\n");
    }

    printf("\n");
    rule_double(stdout);

    return 0;
}
