/// \file CATStack.h
/// \brief Defines a templated stack
/// \ingroup CAT
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef _CATStack_H_
#define _CATStack_H_

#include "CATInternal.h"

/// \class CATStack CATStack.h
/// \brief Defines a templated stack
/// \ingroup CAT
///
/// CATStack does not take any responsibility
/// for object deletion.  It is just a simple object stack.
///
template<class T>
class CATStack
{
   public: 
      /// Enumeration callback for objects
      /// \sa Enumerate
      typedef void (*CATSTACKENUMCB)(T& object, void* userParam);

   protected:
      
      /// \class Node
      /// \brief Protected node class for CATStack
      /// 
      template<class T>
      class Node
      {
         friend CATStack<T>;
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
            T              fData;   // Data ptr
            Node<T>* fNext;   // Ptr to next node
      };
         

   public:
      /// CATStack constructor      
      CATStack()
      {
         fHead = 0;
         fSize = 0;
      }

      /// CATStack copy constructor
      CATStack(const CATStack& srcStack)
      {
         CATUInt32 size = srcStack.Size();
         
         Node<T>* curNode = srcStack.fHead;
         while (curNode)
         {
            this->Push(curNode->fData);
            curNode = curNode->fNext;
         }
      }
      
      /// CATStack virtual destructor
      virtual ~CATStack()
      {
         Clear();
      }

      /// Clear() clears the stack.
      ///
      /// This will cause a leak if the objects
      /// pushed onto the stack are *'s that need deletion.
      void Clear()
      {
         T object;
         while (CATSUCCEEDED(this->Pop(object)));
      }

      /// Operator= override for copying stacks around.
      CATStack& operator=(const CATStack& srcStack)
      {
         // Check for self assignment
         if (&srcStack == this)
            return *this;

         int size = srcStack.Size();
         
         Node<T>* curNode = srcStack.fHead;
         while (curNode)
         {
            this->Push(curNode->fData);
            curNode = curNode->fNext;
         }

         return *this;
      }
            
      /// Push() inserts an object at the head of the stack.
      ///
      /// \param object - Object to be placed in the list
      ///
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa Pop()
      /// 
      CATResult Push(T& object)
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
         
         node->fNext = fHead;
         fHead = node;      
         fSize++;
         return CATRESULT(CAT_SUCCESS);
      }

      /// Pop() pops the next object off of the stack.
      ///
      /// \param object - pointer to receive the object pointer
      /// 
      /// \return CATResult - CAT_SUCCESS on success.
      /// \sa Push(), Size()
      /// 
      CATResult Pop(T& object)
      {
         if (fSize == 0)
         {
            //return CATRESULTDESC(CAT_ERR_STACK_EMPTY,"Stack is empty");
            return CATRESULT(CAT_ERR_STACK_EMPTY);
         }
         Node<T>* node = fHead;
         object = node->fData;
         fHead =  node->fNext;         
         delete node;
         fSize--;
                                       
         return CATRESULT(CAT_SUCCESS);
      }

      /// Size() returns the number of items in the list.
      CATUInt32  Size() const
      {
         return fSize;
      }

      /// Enumerate() calls the specified callback once for each
      /// item on the stack.
      ///
      /// The type of callback must match the specified list type.
      ///
      /// \param enumCallback - Called for each list item in order
      /// \param userParam - user defined parameter, passed back in callback..      
      void Enumerate(CATSTACKENUMCB enumCallback, void* userParam)
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
      CATUInt32     fSize;
};    


#endif // _CATStack_H_
