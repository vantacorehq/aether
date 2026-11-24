#include "aether.h"
#include <stdio.h>

int main(void) {
    AetherWorld w;
    aether_init(&w);

    aether_add_account(&w, "cash");
    aether_add_account(&w, "liability");
    aether_add_account(&w, "equity");

    /* Accounting identity: cash - liability - equity == 0 */
    int32_t identity[] = {1, -1, -1};
    aether_add_constraint(&w, identity, 3, AETHER_EQ, 0);

    /* cash >= 0 */
    int32_t non_negative[] = {1, 0, 0};
    aether_add_constraint(&w, non_negative, 3, AETHER_GE, 0);

    printf("--- valid transition: +100 cash, +100 equity ---\n");
    int64_t d1[] = {100, 0, 100};
    if (aether_apply(&w, d1, 3) == AETHER_OK) {
        printf("accepted\n");
    } else {
        printf("rejected (unexpected)\n");
    }

    printf("\n--- invalid transition: -200 cash ---\n");
    int64_t d2[] = {-200, 0, 0};
    if (aether_apply(&w, d2, 3) == AETHER_REJECTED) {
        printf("correctly rejected\n");
    } else {
        printf("accepted (bug!)\n");
    }

    printf("\n--- valid: pay liability from cash ---\n");
    /* first create some liability */
    int64_t d3[] = {50, 50, 0};
    aether_apply(&w, d3, 3);   /* still balanced */

    int64_t d4[] = {-30, -30, 0};
    if (aether_apply(&w, d4, 3) == AETHER_OK) {
        printf("accepted\n");
    }

    printf("\n");
    aether_print(&w);
    return 0;
}
