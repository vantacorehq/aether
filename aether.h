#ifndef AETHER_H
#define AETHER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hard limits — keep small so the whole engine stays auditable. */
#ifndef AETHER_MAX_ACCOUNTS
#define AETHER_MAX_ACCOUNTS 32
#endif

#ifndef AETHER_MAX_CONSTRAINTS
#define AETHER_MAX_CONSTRAINTS 64
#endif

#ifndef AETHER_MAX_JOURNAL
#define AETHER_MAX_JOURNAL 4096
#endif

#ifndef AETHER_NAME_LEN
#define AETHER_NAME_LEN 32
#endif

typedef enum {
    AETHER_OK = 0,
    AETHER_REJECTED,
    AETHER_FULL,
    AETHER_BAD_ARG
} AetherResult;

typedef enum {
    AETHER_EQ = 0,
    AETHER_GE,
    AETHER_LE
} AetherOp;

typedef struct {
    char     name[AETHER_NAME_LEN];
    int64_t  balance;
} AetherAccount;

typedef struct {
    int32_t  coeffs[AETHER_MAX_ACCOUNTS];
    size_t   n_coeffs;          /* how many accounts this constraint touches */
    AetherOp op;
    int64_t  rhs;
} AetherConstraint;

typedef struct {
    int64_t  deltas[AETHER_MAX_ACCOUNTS];
    size_t   n;
} AetherJournalEntry;

typedef struct {
    AetherAccount     accounts[AETHER_MAX_ACCOUNTS];
    size_t            n_accounts;

    AetherConstraint  constraints[AETHER_MAX_CONSTRAINTS];
    size_t            n_constraints;

    AetherJournalEntry journal[AETHER_MAX_JOURNAL];
    size_t            n_journal;
} AetherWorld;

void         aether_init(AetherWorld *w);

AetherResult aether_add_account(AetherWorld *w, const char *name);

AetherResult aether_add_constraint(AetherWorld *w,
                                   const int32_t *coeffs, size_t n,
                                   AetherOp op, int64_t rhs);

/* Apply a vector of deltas atomically.
 * Either every constraint still holds after the deltas, or nothing changes.
 */
AetherResult aether_apply(AetherWorld *w,
                          const int64_t *deltas, size_t n);

int64_t      aether_balance(const AetherWorld *w, size_t idx);

void         aether_print(const AetherWorld *w);

#ifdef __cplusplus
}
#endif

#endif /* AETHER_H */
