/**
  SitePathMD5.h
*/

#ifndef _MODULES_SITEPATHMD5_H_
#define _MODULES_SITEPATHMD5_H_

#include <config.h>

class SitePathMD5 {
public:
	SitePathMD5(uint64_t site_md5, uint64_t path_md5): site_md5(site_md5), path_md5(path_md5) {};
	~SitePathMD5() {};

	uint64_t site_md5;
	uint64_t path_md5;
};

struct SitePathMD5_hash: public std::unary_function<SitePathMD5*, size_t> {
	/** hash function for the full_md5 type */
        size_t operator() (const SitePathMD5 &fm) const {
                return std::tr1::hash<uint64_t>()(fm.site_md5+fm.path_md5);
        }
};

struct SitePathMD5_equal {
        bool operator()(const SitePathMD5 &fm1, const SitePathMD5 &fm2) const {
                return (fm1.site_md5 == fm2.site_md5 && fm1.path_md5 == fm2.path_md5);
        }
};

#endif
