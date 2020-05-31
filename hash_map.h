#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <cstdint>

#ifdef _DEBUG
#include <string.h>
#include <iostream>
#endif

/*

A custom implementation of a hashed mapping from an int32_t into a value.
The purposes of creating this in descending value:
   1. To learn how to implement hash maps.
   2. To practice memory management.
   3. For actual use.

The hope is the implementation will be faster than the std implementations.
It will attain this speed by not having the overhead of abstraction the standard
library needs to satisfy any/all needs. This only needs to do a subset of the functionality.

The data is organized into a series of linked lists of hash_map_ll_nodes.
Each hashed index has its own unique linked list. To access an element,
an index will be provided to the access operator []. This index will
be hashed to find the associated linked list. Then the list will be traversed
to find the original provided index which is stored as part of the node along
with the corresponding element.

Hashed Index -> First node of hash_map_ll_node linked list.
-------------
[0] -> nullptr
[1] -> hash_map_ll_node -> nullptr
[2] -> hash_map_ll_node -> hash_map_ll_node -> nullptr
[3] -> hash_map_ll_node -> hash_map_ll_node -> ... -> nullptr
...

*/

namespace rstd
{

  namespace rstd_support
  {
    template< typename T>
    class hash_map_ll_node final
    {

    public:

      //! The raw key of the hash map to be accessed.
      int32_t rawKey = 0;

      //! The data stored at the raw key.
      T* dataPointer = nullptr;

      //! The parent node in the linked list.
      hash_map_ll_node* parentNode = nullptr;

      //! The child node in the linked list.
      hash_map_ll_node* childNode = nullptr;

      hash_map_ll_node( int32_t rawkey, hash_map_ll_node* parent, T* data ) :
        rawKey( rawkey ),
        dataPointer( data ),
        parentNode( parent ) {}

      hash_map_ll_node( const hash_map_ll_node& other, hash_map_ll_node* parent = nullptr ) :
        rawKey( other.rawKey ),
        dataPointer( new T( *( other.dataPointer ) ) ),
        parentNode( parent ) {

        if ( other.childNode != nullptr ) {
          childNode = new hash_map_ll_node( *( other.childNode ), this );
        }
      }

      hash_map_ll_node( hash_map_ll_node&& other ) = delete;
      hash_map_ll_node& operator=( const hash_map_ll_node& other ) = delete;
      hash_map_ll_node& operator=( hash_map_ll_node&& other ) = delete;

      ~hash_map_ll_node() {
        delete dataPointer;
        delete childNode;
      }

    };
  }

  template< typename T >
  class hash_map
  {

  private:

    //! The number of hashed indices representing the linked lists.
    uint32_t _hashIndices = 0;

    //! The number of elements in the map.
    uint32_t _elementCount = 0;

    //! Pointer to the hash_map_ll_node linked list array.
    rstd_support::hash_map_ll_node<T>** _heads = nullptr;

  public:

    template< typename valtype >
    friend void swap( hash_map<valtype>& map1, hash_map<valtype>& map2 );

    hash_map( uint32_t storageSize = 256 ) {

      _hashIndices = storageSize - 1;
      _hashIndices |= _hashIndices >> 1;
      _hashIndices |= _hashIndices >> 2;
      _hashIndices |= _hashIndices >> 4;
      _hashIndices |= _hashIndices >> 8;
      _hashIndices |= _hashIndices >> 16;
      _hashIndices++;

      if ( _hashIndices == 0 ) {
        _hashIndices = 256;
      }

      _heads = new rstd_support::hash_map_ll_node<T> * [_hashIndices];
      memsetHeads();
    }

    hash_map( const hash_map& other ) : hash_map( other._hashIndices ) {

      for ( uint32_t i = 0; i < _hashIndices; i++ ) {
        if ( other._heads[i] != nullptr ) {
          _heads[i] = new rstd_support::hash_map_ll_node<T>( *( other._heads[i] ) );
        }
      }
    }

    hash_map( hash_map&& other ) noexcept {
      swap( *this, other );
    }

    hash_map& operator=( const hash_map& other ) {
      if ( this != &other ) {
        hash_map temp( other );
        swap( *this, temp );
      }
      return *this;
    }

    hash_map& operator=( hash_map&& other ) noexcept {
      swap( *this, other );
      return *this;
    }

    //! Clean up, aisle hash_map.
    virtual ~hash_map() {
      clear();
      delete[] _heads;
    }

    //! Accessor operator.
    T& operator[]( int32_t rawKey ) {

      uint32_t hashedKey = hashThis( rawKey );
      rstd_support::hash_map_ll_node<T>* curNode = _heads[hashedKey];

      //First record
      if ( curNode == nullptr ) {
        _heads[hashedKey] = new rstd_support::hash_map_ll_node<T>( rawKey, nullptr, new T() );
        _elementCount++;
        return *( _heads[hashedKey]->dataPointer );
      }

      do {

        if ( curNode->rawKey == rawKey ) {
          return *( curNode->dataPointer );
        }

        if ( curNode->childNode != nullptr ) {
          curNode = curNode->childNode;
          continue;
        }

        curNode->childNode = new rstd_support::hash_map_ll_node<T>( rawKey, curNode, new T() );
        _elementCount++;
        return *( curNode->childNode->dataPointer );

      } while ( true );
    }

    //! Clears out the hash map from items.
    void clear() {

      for ( uint32_t i = 0; i < _hashIndices; i++ ) {

        rstd_support::hash_map_ll_node<T>* toDelete = _heads[i];
        rstd_support::hash_map_ll_node<T>* nextToDelete;

        while ( toDelete != nullptr ) {
          nextToDelete = toDelete->childNode;
          delete toDelete;
          toDelete = nextToDelete;
        }
      }

      memsetHeads();
    }

    //! Erases a single entity from the map.
    void erase( int32_t rawKey ) {

      uint32_t hashedKey = hashThis( rawKey );
      rstd_support::hash_map_ll_node<T>* curNode = _heads[hashedKey];

      if ( curNode == nullptr ) {
        return;
      }

      do {

        if ( curNode->rawKey == rawKey ) {

          if ( curNode->parentNode == nullptr ) {
            _heads[hashedKey] = curNode->childNode;
          }
          else {
            curNode->parentNode->childNode = curNode->childNode;
          }
          delete curNode;
          return;
        }

        if ( curNode->childNode == nullptr ) {
          return;
        }
        curNode = curNode->childNode;

      } while ( true );

    }

    //! Checks if the map is empty or not.
    bool empty() {
      return _elementCount == 0;
    }

    //! Returns the number of elements in the map.
    uint32_t size() {
      return _elementCount;
    }

    //! Returns the density of the bucket containing the given key.
    uint32_t bucketDensity( int32_t rawKey ) {

      uint32_t counter = 0;
      rstd_support::hash_map_ll_node<T>* curNode = _heads[hashThis( rawKey )];

      while ( curNode != nullptr ) {
        counter++;
        curNode = curNode->childNode;
      }

      return counter;
    }

    //! Returns the number of buckets (hash size?).
    uint32_t bucketCount() {
      return _hashIndices;
    }

#ifdef _DEBUG

    //! Prints each entry in map to console. Mainly a debug thing.
    void debugString() {

      std::cout << "Loc\tKey\tValue" << std::endl;
      for ( size_t i = 0; i < _hashIndices; i++ ) {

        if ( headerHasData( i ) ) {

          rstd_support::hash_map_ll_node<T>* h = _heads[i];

          while ( h != nullptr ) {
            std::cout << i << "\t" << ( h->rawKey ) << "\t" << *( h->dataPointer ) << std::endl;
            h = h->childNode;
          }
        }
      }
    }

#endif

  private:

    //! Clears the _heads data to all 0 bits.
    void memsetHeads() {
      memset( _heads, 0, sizeof( rstd_support::hash_map_ll_node<T>* ) * _hashIndices );
    }

    //! Hashes the key
    uint32_t hashThis( int32_t rawKey ) {
      return (uint32_t) ( rawKey & ( _hashIndices - 1 ) );
    }

    //! Checks to see if the header has a hash_map_ll_node.
    bool headerHasData( uint32_t hashedKey ) {
      return _heads[hashedKey] != nullptr;
    }

  };

  template< typename valtype >
  void swap( hash_map<valtype>& map1, hash_map<valtype>& map2 ) {
    using std::swap;
    swap( map1._hashIndices, map2._hashIndices );
    swap( map1._elementCount, map2._elementCount );
    swap( map1._heads, map2._heads );
  }

}

#endif // HASH_MAP_H


















