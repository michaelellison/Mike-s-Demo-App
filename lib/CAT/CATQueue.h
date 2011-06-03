/// \file CATQueue.h
/// \brief Templated Queue object
/// \ingroup CAT
/// 
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//

#ifndef CATQueue_H_
#define CATQueue_H_

#include "CATInternal.h"

/// \class CATQueue CATQueue.h
/// \brief Defines a templated queue
/// \ingroup CAT
template<class T>
class CATQueue
{
   public: 
      /// Enumeration callback for objects
      /// \sa Enumerate
      typedef void (*CATQueueENUMCB)(T& object, void* userParam);

   protected:
      
      /// \class Node
      /// \brief Protected node class for CATQueue
      /// 
      template<class T>
      class Node
      {
         friend CATQueue<T>;
         protected:              
            Node(T& data)
            {
               fData    = data;
               fNext    = 0;
            }

            // Node destructor - when a node is destroyed,
            // it may destroy the data it contains if the list
            // owns it.
            virtual ~Node()
            {
            }

         private:
            T        fData;   // Data ptr
            Node<T>* fNext;   // Ptr to next node
      };
         

   public:
      /// CATQueue constructor      
      CATQueue()
      {
         fHead = 0;
         fTail = 0;
         fSize = 0;
      }

      /// CATQueue copy constructor
      CATQueue(const CATQueue& srcQueue)
      {
         int size = srcQueue.Size();
         
         Node<T>* curNode = srcQueue.fHead;
         while (curNode)
         {
            this->Queue(curNode->fData);
            curNode = curNode->fNext;
         }
      }
      
      /// CATQueue virtual destructor
      virtual ~CATQueue()
      {
         Clear();
      }

      /// Clear() clears the queue.
      ///
      /// This will cause a leak if the objects
      /// queueed onto the queue are *'s that need deletion.
      void Clear()
      {
         T object;
         while (CATSUCCEEDED(this->Next(object)));
      }

      /// Operator= override for copying queues around.
      CATQueue& operator=(const CATQueue& srcQueue)
      {
         // Check for self assignment
         if (&srcQueue == this)
            return *this;

         int size = srcQueue.Size();
         
         Node<T>* curNode = srcQueue.fHead;
         while (curNode)
         {
            this->Queue(curNode->fData);
            curNode = curNode->fNext;
         }

         return *this;
      }
            
      /// Queue() inserts an object at the end of the queue.
      ///
      /// \param object - Object to be placed in the queue
      ///
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa Next()
      /// 
      CATResult Queue(T& object)
      {
         Node<T>* node = 0;
         try
         {
            node = new Node<T>(object);
         }
         catch(...)
         {
            // We aren't throwing exceptions from within the Node
            // constructor, so the only exception here that we might
            // expect would be one from a new handler.  This is a compiler
            // specific issue though - it might throw, it might just return
            // 0.  We'll catch it either way in the next check.
            node = 0;
         }

         if (node == 0)
         {
            return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
         }                  
         
         if (fTail == 0)
         {
            CATASSERT(fHead == 0, "Tail is null, but head isn't. Error in queue.");
            fHead = node;
            fTail = node;
         }
         else
         {  
            fTail->fNext = node;
            fTail = node;
         }          
            
         fSize++;
         return CATRESULT(CAT_SUCCESS);
      }

      /// Next() dequeues the next object off of the queue.
      ///
      /// \param object - pointer to receive the object pointer
      /// 
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa Queue(), Size()
      /// 
      CATResult Next(T& object)
      {
         if (fSize == 0)
         {
            return CATRESULT(CAT_ERR_QUEUE_EMPTY);
         }
         Node<T>* node = fHead;
         object = node->fData;
         fHead =  node->fNext;         
         delete node;
         fSize--;

         if (fHead == 0)
         {
            fTail = 0;
         }
                                       
         return CATRESULT(CAT_SUCCESS);
      }

      /// Size() returns the number of items in the list.
      CATUInt32  Size() const
      {
         return fSize;
      }

      /// Enumerate() calls the specified callback once for each
      /// item on the queue.
      ///
      /// The type of callback must match the specified list type.
      ///
      /// \param enumCallback - Called for each list item in order
      /// \param userParam - user defined parameter, passed back in callback..      
      void Enumerate(CATQueueENUMCB enumCallback, void* userParam)
      {
         CATASSERT(enumCallback != 0, "Callback must be valid.");
         
         if (!enumCallback)
           return;

         Node<T>* curNode = fHead;         
         while (curNode != 0)
         {
            enumCallback(curNode->fData,userParam);
            curNode = curNode->fNext;          
         }
      }


   private:
      Node<T>*    fHead;   // Head ptr - null if empty
      Node<T>*    fTail;   // Tail ptr - null if empty
      CATUInt32     fSize;
};    



#endif // CATQueue_H_
