#ifndef __MAPPER_H__
#define __MAPPER_H__


#ifdef __cplusplus
extern "C" {
#endif

void load_map(const char *map_file);
void unload_map();
const char *dispatch_kc(int, int);

#ifdef __cplusplus
}
#endif

#endif
