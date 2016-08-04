#ifndef __JITTERENTROPY_H__
#define __JITTERENTROPY_H__

/***************************************************************************
 * Helper functions
 ***************************************************************************/

void jent_get_nstime(__u64 *out);
__u64 jent_rol64(__u64 word, unsigned int shift);
void *jent_zalloc(unsigned int len);
void jent_zfree(void *ptr);
int jent_fips_enabled(void);
void jent_panic(char *s);
void jent_memcpy(void *dest, const void *src, unsigned int n);

struct rand_data;
int jent_read_entropy(struct rand_data *ec, unsigned char *data,
		      unsigned int len);
int jent_entropy_init(void);
struct rand_data *jent_entropy_collector_alloc(unsigned int osr,
					       unsigned int flags);
void jent_entropy_collector_free(struct rand_data *entropy_collector);


#endif
