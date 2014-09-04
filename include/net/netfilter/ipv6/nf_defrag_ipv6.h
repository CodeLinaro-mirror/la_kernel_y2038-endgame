#ifndef _NF_DEFRAG_IPV6_H
#define _NF_DEFRAG_IPV6_H

#if defined(CONFIG_NF_DEFRAG_IPV6) || (defined(CONFIG_NF_DEFRAG_IPV6_MODULE) && defined(MODULE))
void nf_defrag_ipv6_enable(void);
#else
static inline void nf_defrag_ipv6_enable(void) {}
#endif

int nf_ct_frag6_init(void);
void nf_ct_frag6_cleanup(void);
struct sk_buff *nf_ct_frag6_gather(struct sk_buff *skb, u32 user);
void nf_ct_frag6_consume_orig(struct sk_buff *skb);

struct inet_frags_ctl;

#endif /* _NF_DEFRAG_IPV6_H */
