#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __OMX_TSEM_H__
#define __OMX_TSEM_H__

#include <pthread.h>

typedef struct omx_sem_t
{
	pthread_cond_t 	condition;
	pthread_mutex_t mutex;
	unsigned int 	semval;
}omx_sem_t;

int omx_sem_init(omx_sem_t* tsem, unsigned int val);
void omx_sem_deinit(omx_sem_t* tsem);
void omx_sem_down(omx_sem_t* tsem);
void omx_sem_up(omx_sem_t* tsem);
void omx_sem_reset(omx_sem_t* tsem);
void omx_sem_wait(omx_sem_t* tsem);
void omx_sem_signal(omx_sem_t* tsem);


#endif //__OMX_TSEM_H__

#ifdef __cplusplus
}
#endif /* __cplusplus */

