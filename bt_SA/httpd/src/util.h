#ifndef HTTPD_UTIL_H
#define HTTPD_UTIL_H


extern char*	IsDefaultRouteExisted( void );
extern char*	IsDefaultRouteExisted2(char* sGateway);
extern int get_tid(void);

extern int parseCfgIdx(char *name, int *cfgID, int *idx1, int *idx2, int *enc);
extern int arccfgFormat(char *cfgname);
extern int parse_cname_idx(char *cname, int *idx_x, int *idx_y);

extern struct words_s *wordslist_init(char *buf, struct words_s *wordlist, int max);

extern int Arc_encode(void *r, unsigned char* buf);
extern int Arc_decode(void *r, unsigned char* buf);

extern int random_str(char*out_buf, int size);

extern char* memstr(char *begin, const char *end, const char *key);
extern int find_term(char **begin, char **end, char *boundary);
extern int is_file_term(const char *begin, char *name, char *fname); 
extern int is_plain_term(const char *begin, char *name);
extern int skip_blank_line(char **begin, char **end);
extern int get_message_from_owl(char *buf, char *recvline, int len);
#endif /*HTTPD_UTIL_H */


