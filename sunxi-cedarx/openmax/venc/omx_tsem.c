/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 Ning Fang <fangning@allwinnertech.com>
*
* This file is part of Cedarx.
*
* Cedarx is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "omx_tsem.h"


/** Initializes the semaphore at a given value
 *
 * @param tsem the semaphore to initialize
 * @param val the initial value of the semaphore
 *
 */
int omx_sem_init(omx_sem_t* tsem, unsigned int val)
{
	int i;

	i = pthread_cond_init(&tsem->condition, NULL);
	if (i!=0)
		return -1;

	i = pthread_mutex_init(&tsem->mutex, NULL);
	if (i!=0)
		return -1;

	tsem->semval = val;

	return 0;
}

/** Destroy the semaphore
 *
 * @param tsem the semaphore to destroy
 */
void omx_sem_deinit(omx_sem_t* tsem)
{
	pthread_cond_destroy(&tsem->condition);
	pthread_mutex_destroy(&tsem->mutex);
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 *
 * @param tsem the semaphore to decrease
 */
void omx_sem_down(omx_sem_t* tsem)
{
	pthread_mutex_lock(&tsem->mutex);

	while (tsem->semval == 0)
	{
		pthread_cond_wait(&tsem->condition, &tsem->mutex);
	}

	tsem->semval--;
	pthread_mutex_unlock(&tsem->mutex);
}

/** Increases the value of the semaphore
 *
 * @param tsem the semaphore to increase
 */
void omx_sem_up(omx_sem_t* tsem)
{
	pthread_mutex_lock(&tsem->mutex);

	tsem->semval++;
	pthread_cond_signal(&tsem->condition);

	pthread_mutex_unlock(&tsem->mutex);
}

/** Reset the value of the semaphore
 *
 * @param tsem the semaphore to reset
 */
void omx_sem_reset(omx_sem_t* tsem)
{
	pthread_mutex_lock(&tsem->mutex);

	tsem->semval=0;

	pthread_mutex_unlock(&tsem->mutex);
}

/** Wait on the condition.
 *
 * @param tsem the semaphore to wait
 */
void omx_sem_wait(omx_sem_t* tsem)
{
	pthread_mutex_lock(&tsem->mutex);

	pthread_cond_wait(&tsem->condition, &tsem->mutex);

	pthread_mutex_unlock(&tsem->mutex);
}

/** Signal the condition,if waiting
 *
 * @param tsem the semaphore to signal
 */
void omx_sem_signal(omx_sem_t* tsem)
{
	pthread_mutex_lock(&tsem->mutex);

	pthread_cond_signal(&tsem->condition);

	pthread_mutex_unlock(&tsem->mutex);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

