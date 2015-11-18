
/**
****************************************************************************************
*
* @file queue.c
*
* @brief Software for queues and threads.
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

#include "queue.h"


QueueRecord SPIRxQueue; //Queues SPI_Rx -> Main 
QueueRecord SPITxQueue; //Queues SPI_Tx -> Main 


void EnQueue(QueueRecord *rec,void *vdata, unsigned short bsize)
{
  struct QueueStorage *tmp;
  tmp=(struct QueueStorage *) malloc(sizeof(struct QueueStorage));
  tmp->Next=NULL;
  tmp->Data=vdata;
	tmp->bLength = bsize;
  if(rec->First==NULL) {
    rec->First=tmp;
    rec->Last=tmp;
  } else {
    rec->Last->Next=tmp;
    rec->Last=tmp;
  }
}

struct QueueElement *DeQueue(QueueRecord *rec)
{
  struct QueueElement *tmp;
  struct QueueStorage *tmpqe;
  if(rec->First==NULL)
    return NULL;
  tmpqe=rec->First;
  rec->First=tmpqe->Next;
  tmp->Data=tmpqe->Data;
	tmp->bLength=tmpqe->bLength;
  free(tmpqe);
  return tmp;
}
