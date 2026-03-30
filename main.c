#include <stdio.h>
#include <string.h>
#include <time.h>

#define NUM_ROOMS    4
#define NAME_LEN     64
#define RECEIPT_FILE "rent_breakdown.txt"

typedef struct {
    char   name[NAME_LEN];
    double room_sqft;
    double share;
    double rent_owed;
    double util_share;
    double total_owed;
} Tenant;

/* ---- helpers ---- */
static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void print_divider(FILE *f)
{
    fprintf(f, "============================================================\n");
}

static void print_thin(FILE *f)
{
    fprintf(f, "------------------------------------------------------------\n");
}

/* Print the full breakdown to any FILE* (stdout or a file) */
static void print_report(FILE *f, Tenant tenants[], int n,
                          double total_rent, double total_sqft,
                          double total_util, const char *timestamp)
{
    int i;

    print_divider(f);
    fprintf(f, "       APARTMENT RENT SPLITTER  (Weighted by Room Size)\n");
    print_divider(f);
    fprintf(f, "  Generated : %s\n", timestamp);
    fprintf(f, "  Monthly Rent        : $%.2f\n", total_rent);
    fprintf(f, "  Monthly Utilities   : $%.2f\n", total_util);
    fprintf(f, "  Total Apartment     : %.1f sq ft\n", total_sqft);
    print_thin(f);

    /* Header */
    fprintf(f, "  %-18s  %8s  %7s  %9s  %9s  %11s\n",
            "Tenant", "Sq Ft", "Share",
            "Rent", "Utils", "TOTAL");
    print_thin(f);

    for (i = 0; i < n; i++) {
        fprintf(f, "  %-18s  %8.1f  %6.1f%%  $%8.2f  $%8.2f  $%10.2f\n",
                tenants[i].name,
                tenants[i].room_sqft,
                tenants[i].share * 100.0,
                tenants[i].rent_owed,
                tenants[i].util_share,
                tenants[i].total_owed);
    }

    print_thin(f);

    /* Column totals */
    double sum_rent = 0.0, sum_util = 0.0, sum_total = 0.0;
    for (i = 0; i < n; i++) {
        sum_rent  += tenants[i].rent_owed;
        sum_util  += tenants[i].util_share;
        sum_total += tenants[i].total_owed;
    }
    fprintf(f, "  %-18s  %8s  %7s  $%8.2f  $%8.2f  $%10.2f\n",
            "TOTAL", "", "", sum_rent, sum_util, sum_total);

    print_divider(f);
    fprintf(f, "\nSUMMARY — What each tenant owes this month:\n\n");
    for (i = 0; i < n; i++) {
        fprintf(f, "  %-18s  rent $%.2f  +  utils $%.2f  =  TOTAL $%.2f\n",
                tenants[i].name,
                tenants[i].rent_owed,
                tenants[i].util_share,
                tenants[i].total_owed);
    }
    fprintf(f, "\n");
    print_divider(f);
}

/* ---- main ---- */
int main(void)
{
    Tenant tenants[NUM_ROOMS];
    double total_rent  = 0.0;
    double total_sqft  = 0.0;
    double total_util  = 0.0;
    int    i;

    /* Timestamp for the receipt */
    time_t now = time(NULL);
    char   timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    print_divider(stdout);
    printf("       APARTMENT RENT SPLITTER  (Weighted by Room Size)\n");
    print_divider(stdout);
    printf("\nRent is split by room size — bigger room, bigger share.\n");
    printf("Utilities are split equally among all tenants.\n\n");

    /* ---- Total rent ---- */
    printf("Enter total monthly RENT ($): ");
    while (scanf("%lf", &total_rent) != 1 || total_rent <= 0.0) {
        printf("  Invalid. Enter a positive number: ");
        clear_input();
    }
    clear_input();

    /* ---- Enhancement 1: Utilities ---- */
    printf("Enter total monthly UTILITIES (electricity + water + internet) ($): ");
    while (scanf("%lf", &total_util) != 1 || total_util < 0.0) {
        printf("  Invalid. Enter 0 or a positive number: ");
        clear_input();
    }
    clear_input();

    print_thin(stdout);
    printf("\nEnter the tenant name and room size for each of the %d rooms.\n\n",
           NUM_ROOMS);

    /* ---- Tenant info ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        printf("--- Room %d ---\n", i + 1);

        printf("  Tenant name      : ");
        fgets(tenants[i].name, NAME_LEN, stdin);
        /* strip trailing newline */
        tenants[i].name[strcspn(tenants[i].name, "\n")] = '\0';
        /* default name if blank */
        if (tenants[i].name[0] == '\0')
            snprintf(tenants[i].name, NAME_LEN, "Tenant %d", i + 1);

        printf("  Room size (sq ft): ");
        while (scanf("%lf", &tenants[i].room_sqft) != 1 ||
               tenants[i].room_sqft <= 0.0) {
            printf("  Invalid size. Enter a positive number: ");
            clear_input();
        }
        clear_input();

        total_sqft += tenants[i].room_sqft;
        printf("\n");
    }

    /* ---- Weighted rent calculation ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        tenants[i].share     = tenants[i].room_sqft / total_sqft;
        tenants[i].rent_owed = tenants[i].share * total_rent;
        /* utilities split equally */
        tenants[i].util_share = total_util / NUM_ROOMS;
    }

    /* FIX: apply rounding correction BEFORE printing so table and summary match */
    double running_rent = 0.0;
    double running_util = 0.0;
    for (i = 0; i < NUM_ROOMS - 1; i++) {
        running_rent += tenants[i].rent_owed;
        running_util += tenants[i].util_share;
    }
    tenants[NUM_ROOMS - 1].rent_owed  = total_rent - running_rent;
    tenants[NUM_ROOMS - 1].util_share = total_util - running_util;

    /* Compute combined totals */
    for (i = 0; i < NUM_ROOMS; i++)
        tenants[i].total_owed = tenants[i].rent_owed + tenants[i].util_share;

    /* ---- Print report to stdout ---- */
    printf("\n");
    print_report(stdout, tenants, NUM_ROOMS,
                 total_rent, total_sqft, total_util, timestamp);

    /* ---- Enhancement 2: Save receipt to file ---- */
    FILE *fp = fopen(RECEIPT_FILE, "w");
    if (fp != NULL) {
        print_report(fp, tenants, NUM_ROOMS,
                     total_rent, total_sqft, total_util, timestamp);
        fclose(fp);
        printf("\nReceipt saved to: %s\n\n", RECEIPT_FILE);
    } else {
        printf("\nNote: Could not save receipt file.\n\n");
    }

    return 0;
}
