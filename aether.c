#include "aether.h"

#include <stdio.h>
#include <string.h>

void aether_init(AetherWorld *w) {
    if (!w) return;
    memset(w, 0, sizeof(*w));
}

AetherResult aether_add_account(AetherWorld *w, const char *name) {
    if (!w || !name) return AETHER_BAD_ARG;
    if (w->n_accounts >= AETHER_MAX_ACCOUNTS) return AETHER_FULL;

    AetherAccount *a = &w->accounts[w->n_accounts];
    strncpy(a->name, name, AETHER_NAME_LEN - 1);
    a->name[AETHER_NAME_LEN - 1] = '\0';
    a->balance = 0;
    w->n_accounts++;
    return AETHER_OK;
}

AetherResult aether_add_constraint(AetherWorld *w,
                                   const int32_t *coeffs, size_t n,
                                   AetherOp op, int64_t rhs) {
    if (!w || !coeffs) return AETHER_BAD_ARG;
    if (n == 0 || n > w->n_accounts) return AETHER_BAD_ARG;
    if (w->n_constraints >= AETHER_MAX_CONSTRAINTS) return AETHER_FULL;
    if (op != AETHER_EQ && op != AETHER_GE && op != AETHER_LE) return AETHER_BAD_ARG;

    AetherConstraint *c = &w->constraints[w->n_constraints];
    memset(c->coeffs, 0, sizeof(c->coeffs));
    for (size_t i = 0; i < n; i++) {
        c->coeffs[i] = coeffs[i];
    }
    c->n_coeffs = n;
    c->op = op;
    c->rhs = rhs;
    w->n_constraints++;
    return AETHER_OK;
}

static int constraint_holds(const AetherWorld *w, const AetherConstraint *c,
                            const int64_t *trial_balances) {
    int64_t sum = 0;
    for (size_t i = 0; i < c->n_coeffs; i++) {
        sum += (int64_t)c->coeffs[i] * trial_balances[i];
    }

    switch (c->op) {
        case AETHER_EQ: return sum == c->rhs;
        case AETHER_GE: return sum >= c->rhs;
        case AETHER_LE: return sum <= c->rhs;
        default:        return 0;
    }
}

AetherResult aether_apply(AetherWorld *w, const int64_t *deltas, size_t n) {
    if (!w || !deltas) return AETHER_BAD_ARG;
    if (n != w->n_accounts) return AETHER_BAD_ARG;
    if (w->n_journal >= AETHER_MAX_JOURNAL) return AETHER_FULL;

    /* Build trial balances */
    int64_t trial[AETHER_MAX_ACCOUNTS];
    for (size_t i = 0; i < w->n_accounts; i++) {
        trial[i] = w->accounts[i].balance + deltas[i];
    }

    /* Every constraint must still hold */
    for (size_t i = 0; i < w->n_constraints; i++) {
        if (!constraint_holds(w, &w->constraints[i], trial)) {
            return AETHER_REJECTED;
        }
    }

    /* Accept */
    for (size_t i = 0; i < w->n_accounts; i++) {
        w->accounts[i].balance = trial[i];
    }

    AetherJournalEntry *e = &w->journal[w->n_journal];
    memset(e, 0, sizeof(*e));
    for (size_t i = 0; i < n; i++) {
        e->deltas[i] = deltas[i];
    }
    e->n = n;
    w->n_journal++;

    return AETHER_OK;
}

int64_t aether_balance(const AetherWorld *w, size_t idx) {
    if (!w || idx >= w->n_accounts) return 0;
    return w->accounts[idx].balance;
}

void aether_print(const AetherWorld *w) {
    if (!w) return;

    printf("=== AETHER world ===\n");
    printf("Accounts (%zu):\n", w->n_accounts);
    for (size_t i = 0; i < w->n_accounts; i++) {
        printf("  [%zu] %-16s  %lld\n",
               i, w->accounts[i].name, (long long)w->accounts[i].balance);
    }

    printf("Constraints (%zu):\n", w->n_constraints);
    for (size_t i = 0; i < w->n_constraints; i++) {
        const AetherConstraint *c = &w->constraints[i];
        printf("  #%zu  ", i);
        for (size_t j = 0; j < c->n_coeffs; j++) {
            if (c->coeffs[j] == 0) continue;
            printf("%+d*%s ", c->coeffs[j], w->accounts[j].name);
        }
        const char *op =
            c->op == AETHER_EQ ? "==" :
            c->op == AETHER_GE ? ">=" : "<=";
        printf("%s %lld\n", op, (long long)c->rhs);
    }

    printf("Journal entries: %zu\n", w->n_journal);
}
