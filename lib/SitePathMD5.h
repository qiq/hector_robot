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

	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5() const;
	void SetPathMD5(uint64_t md5);
	uint64_t GetPathMD5() const;

private:
	uint64_t site_md5;
	uint64_t path_md5;
};

inline void SitePathMD5::SetSiteMD5(uint64_t md5) {
	site_md5 = md5;
}

inline uint64_t SitePathMD5::GetSiteMD5() const {
	return site_md5;
}

inline void SitePathMD5::SetPathMD5(uint64_t md5) {
	path_md5 = md5;
}

inline uint64_t SitePathMD5::GetPathMD5() const {
	return path_md5;
}

struct SitePathMD5_hash: public std::unary_function<SitePathMD5*, size_t> {
	/** hash function for the full_md5 type */
        size_t operator() (const SitePathMD5 &fm) const {
                return std::tr1::hash<uint64_t>()(fm.GetSiteMD5()+fm.GetPathMD5());
        }
};

struct SitePathMD5_equal {
        bool operator()(const SitePathMD5 &fm1, const SitePathMD5 &fm2) const {
                return (fm1.GetSiteMD5() == fm2.GetSiteMD5() && fm1.GetPathMD5() == fm2.GetPathMD5());
        }
};

#endif
