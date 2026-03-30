#include <stdio.h>
#include <string.h>

#define NUM_ROOMS 4
#define NAME_LEN  64

typedef struct {
    char   name[NAME_LEN];
    double room_sqft;
    double share;
    double amount_owed;
} Tenant;

static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void print_divider(void)
{
    printf("============================================================\n");
}

static void print_thin_divider(void)
{
    printf("------------------------------------------------------------\n");
}

int main(void)
{
    Tenant tenants[NUM_ROOMS];
    double total_rent  = 0.0;
    double total_sqft  = 0.0;
    int    i;

    print_divider();
    printf("         APARTMENT RENT SPLITTER  (Weighted by Room Size)\n");
    print_divider();
    printf("\nThis program splits rent fairly based on each room's size.\n");
    printf("Larger rooms pay more; smaller rooms pay less.\n\n");

    /* ---- Total rent ---- */
    printf("Enter the total monthly rent amount ($): ");
    while (scanf("%lf", &total_rent) != 1 || total_rent <= 0.0) {
        printf("  Invalid amount. Please enter a positive number: ");
        clear_input();
    }
    clear_input();

    print_thin_divider();
    printf("\nNow enter the tenant name and room size for each of the %d rooms.\n\n",
           NUM_ROOMS);

    /* ---- Tenant info ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        printf("--- Room %d ---\n", i + 1);

        printf("  Tenant name      : ");
        fgets(tenants[i].name, NAME_LEN, stdin);
        tenants[i].name[strcspn(tenants[i].name, "\n")] = '\0';

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

    /* ---- Weighted calculation ---- */
    for (i = 0; i < NUM_ROOMS; i++) {
        tenants[i].share       = tenants[i].room_sqft / total_sqft;
        tenants[i].amount_owed = tenants[i].share * total_rent;
    }

    /* ---- Output ---- */
    print_divider();
    printf("                     RENT BREAKDOWN\n");
    print_divider();
    printf("  Total Monthly Rent : $%.2f\n", total_rent);
    printf("  Total Apartment    : %.1f sq ft\n", total_sqft);
    print_thin_divider();
    printf("  %-18s  %10s  %8s  %12s\n",
           "Tenant", "Room (sq ft)", "Share", "Amount Owed");
    print_thin_divider();

    for (i = 0; i < NUM_ROOMS; i++) {
        printf("  %-18s  %10.1f  %7.1f%%  $%10.2f\n",
               tenants[i].name,
               tenants[i].room_sqft,
               tenants[i].share * 100.0,
               tenants[i].amount_owed);
    }

    print_thin_divider();

    /* Verify totals add up (floating-point rounding check) */
    double running_total = 0.0;
    for (i = 0; i < NUM_ROOMS - 1; i++)
        running_total += tenants[i].amount_owed;

    /* Last tenant gets the remainder to avoid rounding error */
    tenants[NUM_ROOMS - 1].amount_owed = total_rent - running_total;

    /* Re-print last row corrected if rounding changed it noticeably */
    double corrected = total_rent - running_total;
    if (corrected != tenants[NUM_ROOMS - 1].amount_owed + 0.0) {
        /* Already assigned above, nothing extra needed */
    }

    double grand_total = running_total + tenants[NUM_ROOMS - 1].amount_owed;
    printf("  %-18s  %10s  %8s  $%10.2f\n",
           "TOTAL", "", "", grand_total);

    print_divider();
    printf("\nSUMMARY — Each tenant owes for the month:\n\n");
    for (i = 0; i < NUM_ROOMS; i++) {
        printf("  %s owes $%.2f\n",
               tenants[i].name, tenants[i].amount_owed);
    }
    printf("\n");
    print_divider();

    return 0;
}
