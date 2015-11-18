/**
****************************************************************************************
*
* @file queue.h
*
* @brief Header file for queues and threads definitions.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

#ifndef QUEUE_H_
#define QUEUE_H_


#include <stdlib.h>
#include <stddef.h>     // standard definition


//#include "stdtypes.h"     


// Queue stuff.
struct QueueStorage {
  struct QueueStorage *Next;
  void *Data;
	unsigned short  bLength;
};

typedef struct {
  struct QueueStorage *First,*Last;
} QueueRecord;

struct QueueElement{
  unsigned short  bLength;
  void  *Data;
};


extern QueueRecord SPIRxQueue; 
extern QueueRecord SPITxQueue; 

void EnQueue(QueueRecord *rec,void *vdata, unsigned short bsize);
struct QueueElement *DeQueue(QueueRecord *rec);



#endif //QUEUE_H_
