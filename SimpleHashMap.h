#ifndef SIMPLEHASHMAP_H
#define SIMPLEHASHMAP_H

#include <cstddef>
#include <string.h>
#include <exception>

#ifdef SIMPLEHASHMAP_DEBUG
#include <iostream>
#endif

/*

A custom implementation of a hashed mapping from an integer into a value.
The purposes of creating this in descending value:
   1. To learn how to implement hash maps.
   2. To practice memory management.
   3. For actual use.

The hope is the implementation will be faster than the std implementations.
It will attain this speed by not having the overhead of abstraction the standard
library needs to satisfy any/all needs. This only needs to do a subset of the functionality.

*/

template< typename T >
class SimpleHashMap
{
    
    public:
        
        SimpleHashMap(size_t storageSize = 256);
        virtual ~SimpleHashMap();
        
        T& operator[](int rawKey);
        
        void clear();
        void erase(int rawKey);
        bool empty();
        size_t size();
        size_t bucketDensity(int rawKey);
        size_t bucketCount();
        
#ifdef SIMPLEHASHMAP_DEBUG
//Prints each entry in map to console. Mainly a debug thing.
void print();
#endif

    private:
        
        /*
        Each index of the data array will have a linked list of HashNodes.
        This is to handle collisions when two keys hash to the same identity.
        nextNode and hasNextNode will default to nullptr and false respectively,
        because at the time of creation, there will not be another node.
        
        When looking up an element, the linked list at the given hash will be traversed to find
        the correct key.
        */
        struct HashNode{
            int rawKey;
            T* dataPointer;
            HashNode* parentNode;
            HashNode* childNode = nullptr;
            HashNode(int rawKey, HashNode* parent, T* data):
                rawKey(rawKey),
                dataPointer(data),
                parentNode(parent){}
            ~HashNode(){ delete dataPointer; }
        };
        
        //Clears the _heads data to all 0 bits.
        void memsetHeads();
        
        //Hashes the key
        int hashThis(int rawKey);
        
        //Checks to see if the header has a HashNode.
        bool headerHasData(int hashedKey);    
        
        //The storage size of the container housing the key array.
        size_t _storageSize;
        
        size_t _nodeCount = 0;
        
        //Pointer to the HashNodes.
        HashNode** _heads;
        
};

template< typename T >
SimpleHashMap<T>::SimpleHashMap(size_t storageSize){
    
    if(!storageSize && !( storageSize & (storageSize - 1) )){
        storageSize = 256;
    }
    _storageSize = storageSize;
    
    _heads = new HashNode * [_storageSize];
    memsetHeads();
}

template< typename T >
SimpleHashMap<T>::~SimpleHashMap(){
    
    for(size_t i = 0; i < _storageSize; i++){
            
        HashNode* toDelete = _heads[i];
        HashNode* nextToDelete;
        
        do{
            nextToDelete = toDelete->childNode;
            delete toDelete;
            toDelete = nextToDelete;
            
        }while(toDelete != nullptr);
    
    }
    memsetHeads();
}

template< typename T >
void SimpleHashMap<T>::memsetHeads(){
    
    memset(_heads, 0, sizeof(*_heads) * _storageSize);
}

template< typename T >
void SimpleHashMap<T>::clear(){
    
    for(size_t i = 0; i < _storageSize; i++){
        if(headerHasData(i)){
            delete _heads[i];
        }
    }
    memsetHeads();
}

template< typename T >
void SimpleHashMap<T>::erase(int rawKey){
    
    int hashedKey = hashThis(rawKey);
    
    if(!headerHasData(hashedKey)){
        return;
    }
    
    HashNode* h = _heads[hashedKey];
    
    do{
            
        if(h->rawKey == rawKey){
            
            if(h->parentNode == nullptr){
                _heads[hashedKey] = h->childNode;
            }
            else{
                h->parentNode->childNode = h->childNode;
            }
            delete h;
            return;
        }
        
        if(h->childNode == nullptr){
            return;
        }
        h = h->childNode;
        
    }while(true);
    
}

template< typename T >
bool SimpleHashMap<T>::empty(){
    
    return _nodeCount == 0;
}

template< typename T >
size_t SimpleHashMap<T>::size(){
    
    return _nodeCount;
}

template< typename T >
size_t SimpleHashMap<T>::bucketDensity(int rawKey){
    
    size_t counter = 0;
    int hashedKey = hashThis(rawKey);
    
    if(headerHasData(hashedKey)){
        counter++;
        HashNode* h = _heads[hashedKey];
        while(h->childNode != nullptr){
            counter++;
            h = h->childNode;
        }
    }
    return counter;
}

template< typename T >
size_t SimpleHashMap<T>::bucketCount(){
    
    return _storageSize;
}

template< typename T >
int SimpleHashMap<T>::hashThis(int rawKey){
    
    if(rawKey < 0){
        rawKey *= -1;
    }
    
    return (int) (rawKey & (_storageSize - 1));
}

template< typename T >
T& SimpleHashMap<T>::operator[](int rawKey){
    
    int hashedKey = hashThis(rawKey);
    
    //First record
    if(!headerHasData(hashedKey)){
        _heads[hashedKey] = new HashNode(rawKey, nullptr, new T());
        _nodeCount++;
        return *(_heads[hashedKey]->dataPointer);
    }
    
    HashNode* h = _heads[hashedKey];
    do{
            
        if(h->rawKey == rawKey){
            return *(h->dataPointer);
        }
        
        if(h->childNode != nullptr){
            h = h->childNode;
            continue;
        }
        
        h->childNode = new HashNode(rawKey, h, new T());
        h = h->childNode;
        _nodeCount++;
        return *(h->dataPointer);
        
    }while(true);
}

template< typename T >
bool SimpleHashMap<T>::headerHasData(int hashedKey){
    
    return _heads[hashedKey] != nullptr;
}

#ifdef SIMPLEHASHMAP_DEBUG
template< typename T >
void SimpleHashMap<T>::print(){
    
    std::cout<<"Loc\tKey\tValue"<<std::endl;
    for(size_t i = 0; i < _storageSize; i++){
        
        if(headerHasData(i)){
            
            HashNode* h = _heads[i];
            
            while(h != nullptr){
                std::cout<<i<<"\t"<<(h->rawKey)<<"\t"<<*(h->dataPointer)<<std::endl;
                h = h->childNode;
            }
        }
        
    }
}
#endif

#endif // SIMPLEHASHMAP_H


















