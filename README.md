<p align="center">
  <strong>AETHER</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/language-C-blue?style=flat-square" alt="C">
  <img src="https://img.shields.io/badge/deps-zero-brightgreen?style=flat-square" alt="zero deps">
  <img src="https://img.shields.io/badge/license-MIT-blue?style=flat-square" alt="MIT">
</p>

**Pure C. Zero dependencies. Financial state that cannot lie.**

Aether is a tiny invariant engine.  
You declare linear constraints over accounts.  
Every proposed mutation is checked *before* it is applied.  
If the new state would violate any constraint, the transition is rejected.  
No partial writes. No "we will fix it later".

Debits must equal credits.  
No account may go below its floor.  
Sum of a group of accounts must stay inside a corridor.  
Whatever you declare becomes a hard gate.

This is not a database.  
This is not an accounting library that hopes the caller is careful.  
This is the last line of defence that sits under a payment system and refuses to accept an impossible world.

---

## Why this exists

Most ledgers treat invariants as documentation or as soft checks after the fact.  
Aether treats them as the *definition* of a valid state.  
If a transition would produce an invalid state, it simply does not happen.

The implementation is deliberately small and readable.  
One can audit it in an afternoon.  
There is no hidden allocator behaviour, no global state, no third-party code.

---

## Core ideas

- **Accounts** are 64-bit signed balances (minor units).
- **Constraints** are linear forms: `Σ coeffᵢ × balanceᵢ  ⋈  rhs`  
  where `⋈` is `==`, `>=`, or `<=`.
- A **world** holds accounts + constraints + a simple append-only journal of accepted transitions.
- `aether_apply()` either fully accepts a set of deltas or rejects them. Atomic.
- Deterministic. Same sequence of accepted operations → same balances and journal.

---

## Quick start

```c
#include "aether.h"

int main(void) {
    AetherWorld w;
    aether_init(&w);

    /* three accounts: cash, liability, equity */
    aether_add_account(&w, "cash");
    aether_add_account(&w, "liability");
    aether_add_account(&w, "equity");

    /* hard rule: cash - liability - equity == 0  (accounting identity) */
    int32_t coeffs[] = {1, -1, -1};
    aether_add_constraint(&w, coeffs, 3, AETHER_EQ, 0);

    /* also: cash must never go negative */
    int32_t cash_only[] = {1, 0, 0};
    aether_add_constraint(&w, cash_only, 3, AETHER_GE, 0);

    /* valid: receive 100 into cash, increase equity */
    int64_t deltas1[] = {100, 0, 100};
    if (aether_apply(&w, deltas1, 3) != AETHER_OK) {
        /* unreachable in this example */
    }

    /* invalid: try to pull 200 from cash without balancing */
    int64_t deltas2[] = {-200, 0, 0};
    if (aether_apply(&w, deltas2, 3) == AETHER_REJECTED) {
        /* correctly rejected — would break both constraints */
    }

    aether_print(&w);
    return 0;
}
```

Build:

```bash
cc -std=c11 -Wall -Wextra -O2 -o example example.c aether.c
./example
```

---

## API (minimal)

```c
typedef enum {
    AETHER_OK = 0,
    AETHER_REJECTED,
    AETHER_FULL,          /* too many accounts / constraints */
    AETHER_BAD_ARG
} AetherResult;

typedef enum {
    AETHER_EQ,
    AETHER_GE,
    AETHER_LE
} AetherOp;

void          aether_init(AetherWorld *w);
AetherResult  aether_add_account(AetherWorld *w, const char *name);
AetherResult  aether_add_constraint(AetherWorld *w,
                                    const int32_t *coeffs, size_t n,
                                    AetherOp op, int64_t rhs);
AetherResult  aether_apply(AetherWorld *w,
                           const int64_t *deltas, size_t n);
int64_t       aether_balance(const AetherWorld *w, size_t idx);
void          aether_print(const AetherWorld *w);
```

Limits (compile-time, deliberately small for auditability):

- `AETHER_MAX_ACCOUNTS`   = 32
- `AETHER_MAX_CONSTRAINTS` = 64
- `AETHER_MAX_JOURNAL`    = 4096

Raise them if you need more; the code stays the same.

---

## What this is not

- Not a full general ledger with charts of accounts, VAT, multi-currency, etc.
- Not a database. Persistence is the caller's job (or use a WAL from another vantacorehq component).
- Not concurrent. Single-threaded by design. Protect with a mutex if needed.

---

## Philosophy

Most systems discover that money disappeared *after* the fact.  
Aether makes the disappearance impossible to accept in the first place.

If a transition would produce a world that violates the declared laws of value,  
the transition is not part of history.

---

## License

MIT
