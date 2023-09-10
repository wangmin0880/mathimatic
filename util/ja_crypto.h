
#ifndef __JA_CRYPTO_H__
#define __JA_CRYPTO_H__

/**
 * struct ja_compress_alg - compression/decompression algorithm
 * @coa_compress: Compress a buffer of specified length, storing the resulting
 *		  data in the specified buffer. Return the length of the
 *		  compressed data in dlen.
 * @coa_decompress: Decompress the source buffer, storing the uncompressed
 *		    data in the specified buffer. The length of the data is
 *		    returned in dlen.
 *
 * All fields are mandatory.
 */
struct ja_compress_alg {
	int (*coa_compress)(struct ja_crypto_tfm *tfm, const ja_u8 *src,
			    unsigned int slen, ja_u8 *dst, unsigned int *dlen);
	int (*coa_decompress)(struct ja_crypto_tfm *tfm, const ja_u8 *src,
			      unsigned int slen, ja_u8 *dst, unsigned int *dlen);
};


/**
 * struct ja_cipher_alg - single-block symmetric ciphers definition
 * @cia_min_keysize: Minimum key size supported by the transformation. This is
 *		     the smallest key length supported by this transformation
 *		     algorithm. This must be set to one of the pre-defined
 *		     values as this is not hardware specific. Possible values
 *		     for this field can be found via git grep "_MIN_KEY_SIZE"
 *		     include/crypto/
 * @cia_max_keysize: Maximum key size supported by the transformation. This is
 *		    the largest key length supported by this transformation
 *		    algorithm. This must be set to one of the pre-defined values
 *		    as this is not hardware specific. Possible values for this
 *		    field can be found via git grep "_MAX_KEY_SIZE"
 *		    include/crypto/
 * @cia_setkey: Set key for the transformation. This function is used to either
 *	        program a supplied key into the hardware or store the key in the
 *	        transformation context for programming it later. Note that this
 *	        function does modify the transformation context. This function
 *	        can be called multiple times during the existence of the
 *	        transformation object, so one must make sure the key is properly
 *	        reprogrammed into the hardware. This function is also
 *	        responsible for checking the key length for validity.
 * @cia_encrypt: Encrypt a single block. This function is used to encrypt a
 *		 single block of data, which must be @cra_blocksize big. This
 *		 always operates on a full @cra_blocksize and it is not possible
 *		 to encrypt a block of smaller size. The supplied buffers must
 *		 therefore also be at least of @cra_blocksize size. Both the
 *		 input and output buffers are always aligned to @cra_alignmask.
 *		 In case either of the input or output buffer supplied by user
 *		 of the crypto API is not aligned to @cra_alignmask, the crypto
 *		 API will re-align the buffers. The re-alignment means that a
 *		 new buffer will be allocated, the data will be copied into the
 *		 new buffer, then the processing will happen on the new buffer,
 *		 then the data will be copied back into the original buffer and
 *		 finally the new buffer will be freed. In case a software
 *		 fallback was put in place in the @cra_init call, this function
 *		 might need to use the fallback if the algorithm doesn't support
 *		 all of the key sizes. In case the key was stored in
 *		 transformation context, the key might need to be re-programmed
 *		 into the hardware in this function. This function shall not
 *		 modify the transformation context, as this function may be
 *		 called in parallel with the same transformation object.
 * @cia_decrypt: Decrypt a single block. This is a reverse counterpart to
 *		 @cia_encrypt, and the conditions are exactly the same.
 *
 * All fields are mandatory and must be filled.
 */
struct ja_cipher_alg {
	unsigned int cia_min_keysize;
	unsigned int cia_max_keysize;
	int (*cia_setkey)(struct ja_crypto_tfm *tfm, const ja_u8 *key,
	                  unsigned int keylen);
	void (*cia_encrypt)(struct ja_crypto_tfm *tfm, ja_u8 *dst, const ja_u8 *src);
	void (*cia_decrypt)(struct ja_crypto_tfm *tfm, ja_u8 *dst, const ja_u8 *src);
};

typedef struct {
	ja_s64  counter;
} ja_atomic64_t;

#define JA_CONFIG_CRYPTO_STATS
/*
 * struct ja_crypto_istat_aead - statistics for AEAD algorithm
 * @encrypt_cnt:	number of encrypt requests
 * @encrypt_tlen:	total data size handled by encrypt requests
 * @decrypt_cnt:	number of decrypt requests
 * @decrypt_tlen:	total data size handled by decrypt requests
 * @err_cnt:		number of error for AEAD requests
 */
struct ja_crypto_istat_aead {
	ja_atomic64_t encrypt_cnt;
	ja_atomic64_t encrypt_tlen;
	ja_atomic64_t decrypt_cnt;
	ja_atomic64_t decrypt_tlen;
	ja_atomic64_t err_cnt;
};

/*
 * struct ja_crypto_istat_akcipher - statistics for akcipher algorithm
 * @encrypt_cnt:	number of encrypt requests
 * @encrypt_tlen:	total data size handled by encrypt requests
 * @decrypt_cnt:	number of decrypt requests
 * @decrypt_tlen:	total data size handled by decrypt requests
 * @verify_cnt:		number of verify operation
 * @sign_cnt:		number of sign requests
 * @err_cnt:		number of error for akcipher requests
 */
struct ja_crypto_istat_akcipher {
	ja_atomic64_t encrypt_cnt;
	ja_atomic64_t encrypt_tlen;
	ja_atomic64_t decrypt_cnt;
	ja_atomic64_t decrypt_tlen;
	ja_atomic64_t verify_cnt;
	ja_atomic64_t sign_cnt;
	ja_atomic64_t err_cnt;
};

/*
 * struct ja_crypto_istat_cipher - statistics for cipher algorithm
 * @encrypt_cnt:	number of encrypt requests
 * @encrypt_tlen:	total data size handled by encrypt requests
 * @decrypt_cnt:	number of decrypt requests
 * @decrypt_tlen:	total data size handled by decrypt requests
 * @err_cnt:		number of error for cipher requests
 */
struct ja_crypto_istat_cipher {
	ja_atomic64_t encrypt_cnt;
	ja_atomic64_t encrypt_tlen;
	ja_atomic64_t decrypt_cnt;
	ja_atomic64_t decrypt_tlen;
	ja_atomic64_t err_cnt;
};

/*
 * struct ja_crypto_istat_compress - statistics for compress algorithm
 * @compress_cnt:	number of compress requests
 * @compress_tlen:	total data size handled by compress requests
 * @decompress_cnt:	number of decompress requests
 * @decompress_tlen:	total data size handled by decompress requests
 * @err_cnt:		number of error for compress requests
 */
struct ja_crypto_istat_compress {
	ja_atomic64_t compress_cnt;
	ja_atomic64_t compress_tlen;
	ja_atomic64_t decompress_cnt;
	ja_atomic64_t decompress_tlen;
	ja_atomic64_t err_cnt;
};

/*
 * struct ja_crypto_istat_hash - statistics for has algorithm
 * @hash_cnt:		number of hash requests
 * @hash_tlen:		total data size hashed
 * @err_cnt:		number of error for hash requests
 */
struct ja_crypto_istat_hash {
	ja_atomic64_t hash_cnt;
	ja_atomic64_t hash_tlen;
	ja_atomic64_t err_cnt;
};

/*
 * struct crypto_istat_kpp - statistics for KPP algorithm
 * @setsecret_cnt:		number of setsecrey operation
 * @generate_public_key_cnt:	number of generate_public_key operation
 * @compute_shared_secret_cnt:	number of compute_shared_secret operation
 * @err_cnt:			number of error for KPP requests
 */
struct ja_crypto_istat_kpp {
	ja_atomic64_t setsecret_cnt;
	ja_atomic64_t generate_public_key_cnt;
	ja_atomic64_t compute_shared_secret_cnt;
	ja_atomic64_t err_cnt;
};

/*
 * struct crypto_istat_rng: statistics for RNG algorithm
 * @generate_cnt:	number of RNG generate requests
 * @generate_tlen:	total data size of generated data by the RNG
 * @seed_cnt:		number of times the RNG was seeded
 * @err_cnt:		number of error for RNG requests
 */
struct ja_crypto_istat_rng {
	ja_atomic64_t generate_cnt;
	ja_atomic64_t generate_tlen;
	ja_atomic64_t seed_cnt;
	ja_atomic64_t err_cnt;
};

/**
 * struct crypto_alg - definition of a cryptograpic cipher algorithm
 * @cra_flags: Flags describing this transformation. See include/linux/crypto.h
 *	       CRYPTO_ALG_* flags for the flags which go in here. Those are
 *	       used for fine-tuning the description of the transformation
 *	       algorithm.
 * @cra_blocksize: Minimum block size of this transformation. The size in bytes
 *		   of the smallest possible unit which can be transformed with
 *		   this algorithm. The users must respect this value.
 *		   In case of HASH transformation, it is possible for a smaller
 *		   block than @cra_blocksize to be passed to the crypto API for
 *		   transformation, in case of any other transformation type, an
 * 		   error will be returned upon any attempt to transform smaller
 *		   than @cra_blocksize chunks.
 * @cra_ctxsize: Size of the operational context of the transformation. This
 *		 value informs the kernel crypto API about the memory size
 *		 needed to be allocated for the transformation context.
 * @cra_alignmask: Alignment mask for the input and output data buffer. The data
 *		   buffer containing the input data for the algorithm must be
 *		   aligned to this alignment mask. The data buffer for the
 *		   output data must be aligned to this alignment mask. Note that
 *		   the Crypto API will do the re-alignment in software, but
 *		   only under special conditions and there is a performance hit.
 *		   The re-alignment happens at these occasions for different
 *		   @cra_u types: cipher -- For both input data and output data
 *		   buffer; ahash -- For output hash destination buf; shash --
 *		   For output hash destination buf.
 *		   This is needed on hardware which is flawed by design and
 *		   cannot pick data from arbitrary addresses.
 * @cra_priority: Priority of this transformation implementation. In case
 *		  multiple transformations with same @cra_name are available to
 *		  the Crypto API, the kernel will use the one with highest
 *		  @cra_priority.
 * @cra_name: Generic name (usable by multiple implementations) of the
 *	      transformation algorithm. This is the name of the transformation
 *	      itself. This field is used by the kernel when looking up the
 *	      providers of particular transformation.
 * @cra_driver_name: Unique name of the transformation provider. This is the
 *		     name of the provider of the transformation. This can be any
 *		     arbitrary value, but in the usual case, this contains the
 *		     name of the chip or provider and the name of the
 *		     transformation algorithm.
 * @cra_type: Type of the cryptographic transformation. This is a pointer to
 *	      struct crypto_type, which implements callbacks common for all
 *	      transformation types. There are multiple options, such as
 *	      &crypto_skcipher_type, &crypto_ahash_type, &crypto_rng_type.
 *	      This field might be empty. In that case, there are no common
 *	      callbacks. This is the case for: cipher, compress, shash.
 * @cra_u: Callbacks implementing the transformation. This is a union of
 *	   multiple structures. Depending on the type of transformation selected
 *	   by @cra_type and @cra_flags above, the associated structure must be
 *	   filled with callbacks. This field might be empty. This is the case
 *	   for ahash, shash.
 * @cra_init: Initialize the cryptographic transformation object. This function
 *	      is used to initialize the cryptographic transformation object.
 *	      This function is called only once at the instantiation time, right
 *	      after the transformation context was allocated. In case the
 *	      cryptographic hardware has some special requirements which need to
 *	      be handled by software, this function shall check for the precise
 *	      requirement of the transformation and put any software fallbacks
 *	      in place.
 * @cra_exit: Deinitialize the cryptographic transformation object. This is a
 *	      counterpart to @cra_init, used to remove various changes set in
 *	      @cra_init.
 * @cra_u.cipher: Union member which contains a single-block symmetric cipher
 *		  definition. See @struct @cipher_alg.
 * @cra_u.compress: Union member which contains a (de)compression algorithm.
 *		    See @struct @compress_alg.
 * @cra_module: Owner of this transformation implementation. Set to THIS_MODULE
 * @cra_list: internally used
 * @cra_users: internally used
 * @cra_refcnt: internally used
 * @cra_destroy: internally used
 *
 * @stats: union of all possible crypto_istat_xxx structures
 * @stats.aead:		statistics for AEAD algorithm
 * @stats.akcipher:	statistics for akcipher algorithm
 * @stats.cipher:	statistics for cipher algorithm
 * @stats.compress:	statistics for compress algorithm
 * @stats.hash:		statistics for hash algorithm
 * @stats.rng:		statistics for rng algorithm
 * @stats.kpp:		statistics for KPP algorithm
 *
 * The struct crypto_alg describes a generic Crypto API algorithm and is common
 * for all of the transformations. Any variable not documented here shall not
 * be used by a cipher implementation as it is internal to the Crypto API.
 */
#define JA_CRYPTO_MAX_ALG_NAME 128
struct ja_crypto_alg {

	ja_u32 cra_flags;
	unsigned int cra_blocksize;
	unsigned int cra_ctxsize;
	unsigned int cra_alignmask;

	struct ja_list_head cra_list;
	struct ja_list_head cra_users;

	int cra_priority;
	int cra_refcnt;

	char cra_name[JA_CRYPTO_MAX_ALG_NAME];
	char cra_driver_name[JA_CRYPTO_MAX_ALG_NAME];

	const struct crypto_type *cra_type;

	union {
		struct ja_cipher_alg cipher;
		struct ja_compress_alg compress;
	} cra_u;

	int (*cra_init)(struct ja_crypto_tfm *tfm);
	void (*cra_exit)(struct ja_crypto_tfm *tfm);
	void (*cra_destroy)(struct ja_crypto_tfm *alg);

	union {
		struct ja_crypto_istat_aead aead;
		struct ja_crypto_istat_akcipher akcipher;
		struct ja_crypto_istat_cipher cipher;
		struct ja_crypto_istat_compress compress;
		struct ja_crypto_istat_hash hash;
		struct ja_crypto_istat_rng rng;
		struct ja_crypto_istat_kpp kpp;
	} stats;

} CRYPTO_MINALIGN_ATTR;

/*
 * Transforms: user-instantiated objects which encapsulate algorithms
 * and core processing logic.  Managed via crypto_alloc_*() and
 * crypto_free_*(), as well as the various helpers below.
 */

struct ja_crypto_tfm {

	ja_u32 crt_flags;

	int node;

	void (*exit)(struct ja_crypto_tfm *tfm);

	struct ja_crypto_alg *__crt_alg;

};


//struct ja_list_head crypto_alg_list;

#endif
