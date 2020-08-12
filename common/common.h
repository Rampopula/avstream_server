#ifndef __COMMON_H__
#define __COMMON_H__

#define TRUE                	(1)
#define FALSE               	(0)
#define LOG(fmt, args...)   	do {\
                                	fprintf(stdout, "%s-%d: " fmt, __FUNCTION__, __LINE__, ##args);\
                            	} while(0)

#endif /* __COMMON_H__ */
