#ifndef SMARTCACHE_H
#define SMARTCACHE_H

#include "smartpointer.h"
#include <unordered_map>
#include <vector>
#include <list>
#include <set>

namespace gmisc {

template<class keyT>
class smartcache_base;

template<class pointerT, class keyT>
class smartcache;

template<typename cacheKeyT = int>
class smartcachable: public smartable{
friend class smartcache_base<cacheKeyT>;
    smartcache_base<cacheKeyT>* _cache = 0;
    cacheKeyT _elementKey;

    unsigned char _flags = NO_FLAGS;
    enum Flags{
        NO_FLAGS    = 0x00,
        BEEN_REGISTERED  = 0x10,
        CHANGED     = 0x20,
        DELETED     = 0x40,
        FULL        = 0xFF
    };
    
    
    void registerElement(const cacheKeyT& key) {
        _elementKey = key;
        setRegistered(true);
        setChangesFlags(false);
    }
    void setRegistered( bool registered = true ){
        if(registered) _flags |=  BEEN_REGISTERED;
        else           _flags &= FULL - BEEN_REGISTERED;
    }
    void setWasDeleted( bool deleted = true) {
        if(deleted) _flags |=  DELETED;
        else        _flags &= FULL - DELETED;
    }
    void setChangesFlags(bool changed){
        if(changed) _flags |=  CHANGED;
        else        _flags &= FULL - CHANGED;
        if(_cache)  _cache->elementChanged(this, changed);
    }

public:
    virtual ~smartcachable() { detachFromCache(); }
    virtual void safeDelete() { delete this; }
    
    bool isDifferentFromData() const {return !isRegistered() || hadChanged() || wasDeleted();}

    bool isDetached() const {return _cache==0;}
    bool isRegistered() const {return (_flags & BEEN_REGISTERED) && _cache;}
    bool hadChanged()      const {return _flags & CHANGED;}
    bool wasDeleted()      const {return _flags & DELETED;}

    smartcache_base<cacheKeyT>* cache() const {return _cache;}
    const cacheKeyT& key() const {return _elementKey;}

    bool detachFromCache() {
        if(!_cache)
            return false;
        _cache->oneCacheElementHasLostReference(_elementKey);
        _cache = 0;
        setRegistered(false);
        return true;
    }

    bool apply() {
        return _cache->apply(this);
    }
    virtual bool validateApply() const {return true;}

    bool reset() {
        return _cache->reset(this);
    }
    virtual bool validateReset() const {return true;}
    
protected:
    smartcachable(smartcache_base<cacheKeyT>* cache = 0) : _cache(cache) {}

    virtual void reportChange() { 
        setChangesFlags(true); 
    }
    bool setElemKey(const cacheKeyT& key) {
        if(isRegistered())
            return _cache->swapKeys(_elementKey, key, false);

        _elementKey = key;
        return true;
    }
};

/*!
 *
 */
template<class pointerT, class keyT = int>
class smartcache_interface
{
    friend  class smartcache<pointerT,keyT>;
protected:
    virtual bool readData(keyT elementKey, pointerT*& element) = 0;
    virtual int  readSize() {return -1;}
    virtual bool writeData(pointerT* /*element*/)  {return false;}
    virtual bool addData(pointerT* /*element*/, keyT& /*key*/)    {return false;}
    virtual bool deleteData(pointerT* /*element*/) {return false;}
    virtual void cacheChanged() {}

    virtual pointerT* newElement() = 0;
    virtual bool copy(const pointerT* from, pointerT* to){
        if(!from || !to)
            return false;

        (*to) = *from;
        return true;
    }
};


using std::unordered_map;
using std::vector;
using std::list;
using std::set;
/*!
 *  \brief the interface for the basic functions of the smartcache
 *
 * This layer of dependency  allows smartcachable elements to validate or reset them-selves.
 */

template<class keyT = int>
class smartcache_base{
public:
    virtual bool apply(SMARTP<smartcachable<keyT>> element)=0;
    virtual bool reset(SMARTP<smartcachable<keyT>> element)=0;
    virtual void oneCacheElementHasLostReference(keyT elementKey)=0;
    virtual bool swapKeys(const keyT& key1, const keyT& key2, bool swap=true)=0;
    
    void elementChanged(SMARTP<smartcachable<keyT>> e, bool changed=true){
        if(changed){ 
            _changedElements.insert(e);
            cacheChanged();
        }else{
            auto it =_changedElements.find(e);
            if(it != _changedElements.end())
                _changedElements.erase(it);
        }
    }

protected:
    void setCache(smartcachable<keyT>* e) {e->_cache = this;}
    void resetCache(smartcachable<keyT>* e) const {e->_cache = 0;}
    void registerElem(smartcachable<keyT>* e, keyT k) const {e->registerElement(k);}
    void setKey(smartcachable<keyT>* e, keyT k) const {e->_elementKey = k;}
    void setWasDeleted(smartcachable<keyT>* e, bool deleted) const {e->setWasDeleted(deleted);}
    void setChangesFlag(smartcachable<keyT>* e, bool change) const {e->setChangesFlags(change);}
    
    virtual void cacheChanged() const = 0;
    set< SMARTP<smartcachable<keyT>> > _changedElements;
};


/*!
 *
 */
template<class pointerT, class keyT = int>
class smartcache: public smartcache_base<keyT>
{
    unordered_map< keyT, pointerT* > _map;
    vector<SMARTP<pointerT>>   _newElements;
    smartcache_interface<pointerT, keyT>* _interface;

public:
    smartcache(smartcache_interface<pointerT, keyT>* dataInterface = 0): _interface(dataInterface) {}

    virtual ~smartcache() {
        for(auto it = _map.begin(); it!=_map.end(); it++)
            it->second->detachFromCache();
    }

    //-----------------------------------------
    void oneCacheElementHasLostReference(keyT elementKey){
            _map.erase(elementKey);
            for(auto it=_newElements.rbegin(); it!=_newElements.rend(); it++){
                if( (*it)->key() == elementKey){
                    _newElements.erase(std::next(it).base());
                    break;
                }
            }
    }

    //-----------------------------------------
    bool contains(const keyT& elementKey){
        return _map.find(elementKey) != _map.end();
    }

    //_________________________________________
    //-----------------------------------------
    bool apply(){
        bool success = true;
        for(auto it=_map.begin(); it!=_map.end(); it++)
            success &= apply(it->second);

        while(_newElements.size())
            success &= apply(_newElements.at(_newElements.size()-1));

        return success;
    }

    //-----------------------------------------
    void reset(){
        while(_newElements.size())
            reset(_newElements.at(_newElements.size()-1));
        for(auto it=_map.begin(); it!=_map.end(); it++)
            reset(it->second);
    }

    //-----------------------------------------
    virtual bool apply(SMARTP<smartcachable<keyT>> element){
        if(!element->isDifferentFromData())
            return true;
        if(element->cache() && element->cache()!=this)
            return false;
        if(!element->validateApply())
            return false;
        if(element->wasDeleted() && !element->isRegistered())
            return true;

        pointerT* p = dynamic_cast<pointerT*>((smartcachable<keyT>*)element);
        assert(p);

        if(!element->isRegistered()){
            // Add
            if(!addElement(SMARTP<pointerT>(p), false))
                return false;

            for(auto it=_newElements.rbegin(); it!=_newElements.rend(); it++){
                if( (*it) == p){
                    _newElements.erase(std::next(it).base());
                    break;
                }
            }
            return true;
        }
        
        if(element->wasDeleted()){
            return deleteElement(p, false);
        }

            // Update
        if(!writeData(p))
            return false;
        this->setChangesFlag(element, false);
        return true;
    }

    //-----------------------------------------
    virtual bool reset(SMARTP<smartcachable<keyT>> element){
        if(!element->isDifferentFromData())
            return true;
        if(dynamic_cast<smartcache<pointerT, keyT>* >(element->cache()) != this)
            return false;
        if(!element->validateReset())
            return false;

        pointerT* p = dynamic_cast<pointerT*>((smartcachable<keyT>*)element);
        assert(p);

        if(!element->isRegistered()){ //Undo adding
            if(!element->detachFromCache())
                return false;
            
            this->setChangesFlag(element, false);
            this->setWasDeleted(element, true);

            for(auto it=_newElements.rbegin(); it!=_newElements.rend(); it++){
                if( (*it) == p) {
                    _newElements.erase(std::next(it).base());
                    break;
                }
            }

            return true;
        }
        
        if(element->wasDeleted())
            this->setWasDeleted(element,false);

            // Reload element from data;
        pointerT* data;
        if(!readData(element->key(), data))
            return false;
        copy(data, p);
        data->safeDelete();
        this->setChangesFlag(element,false);
        return true;
    }

    //_________________________________________
    //-----------------------------------------
    SMARTP<pointerT> readElementFromCache(keyT elementKey){
        pointerT* p;

        // Try to read element from the cache
        auto it = _map.find(elementKey);
        if(it != _map.end()){
            p = it->second;
            if(p->wasDeleted())
                return SMARTP<pointerT>();

            return SMARTP<pointerT>(p);
        }

        // Read element from data
        if(!readData(elementKey, p))
            return 0;

        SMARTP<pointerT> sp(p);
        emplaceElement(sp);

        return sp;
    }

    //-----------------------------------------
     bool readDetachedElement(keyT elementKey, SMARTP<pointerT>& detachedElement, bool notFromCache=false) const {
        pointerT* p = 0;

        if(!notFromCache){
            // Try to read element from the cache
            auto it = _map.find(elementKey);
            if(it != _map.end()){
                detachedElement = 0;
                if(!it->second || it->second->wasDeleted())
                    return true;

                // create a autonomous copy of the smart pointer
                p = newElement();
                copy(it->second,p);
                detachedElement = SMARTP<pointerT>(p);
                return true;
            }
        }

        // Read element from data
        p = newElement();
        if(!readData(elementKey, p))
            p->safeDelete();
        else
            detachedElement = SMARTP<pointerT>(p);

        return false;
    }

    //-----------------------------------------
    SMARTP<pointerT> readDetachedElement(keyT elementKey, bool notFromCache=false) const {
        SMARTP<pointerT> p;
        readDetachedElement(elementKey, p, notFromCache);
        return p;
    }

    //-----------------------------------------
    int size() const {
        int s = readSize();
        if(s==-1)
            return s;
        s += _newElements.size();
        return s;
    }
    
    //-----------------------------------------
    bool addElement(const SMARTP<pointerT>& element, bool toCache=true){
        if(element->isRegistered())
            return false;

        pointerT* p = (pointerT*)element;

        if(toCache){
            _newElements.insert(_newElements.end(), element);
            this->setCache(element);
        }else{
            // Add
            keyT key;
            if(!addData(p, key))
                return false;
            this->setKey(p, key);
            return emplaceElement(element);
        }
        return true;
    }

    //-----------------------------------------
    bool emplaceElement(const SMARTP<pointerT>& element){
        pointerT* p = (pointerT*)element;
        const keyT& key = p->key();

        if(_map.find(key)!=_map.end())
            return false;

        _map[key] = p;
        this->registerElem(element, key);
        this->setCache(element);
        this->setChangesFlag(element, false);

        return true;
    }

    //-----------------------------------------
    bool deleteElement(const SMARTP<pointerT>& element, bool fromCache=true){
        if(!element->isRegistered() || dynamic_cast<smartcache<pointerT,keyT>*>(element->cache())!=this)
            return false;

        if(fromCache)
            this->setWasDeleted(element, true);
        else{
            pointerT* p = (pointerT*)element;
            if(!deleteData(p))
                return false;
            element->detachFromCache();
            this->setChangesFlag(element, false);
        }
        return true;
    }

    //-----------------------------------------
    bool swapKeys(const keyT &key1, const keyT &key2, bool swap=true){
        auto key1It = _map.end(), key2It = _map.end();

        for(auto it=_map.begin(); it!=_map.end(); it++){
            if(it->first == key1)
                key1It = it;
            else if(it->first == key2)
                key2It = it;

            if(key1It!=_map.end() && key2It!=_map.end())
                break;
        }

        if( key1It!=_map.end() || ( !swap && (key2It!=_map.end()) ))
            return false;

        pointerT* p1 = key1It->second;
        pointerT* p2 = key2It!=_map.end()?key2It->second:0;


        _map[key2] = p1;
        this->setKey(p1, key2);
        if(p2){
            _map[key1] = p2;
            this->setKey(p2, key1);
        }else
            _map.erase(key1It);

        return true;
    }


    //-----------------------------------------
    const vector<SMARTP<pointerT>>& newElements() const{
        return _newElements;
    }

    //__________________________________________________________
    //==========================================================
protected:
    virtual bool readData(keyT elementKey, pointerT*& element) const {
        if(!_interface)
            return false;
        return _interface->readData(elementKey, element);
     }
    virtual int readSize() const { if(!_interface)  return -1;
                                   return _interface->readSize(); }
    virtual bool writeData(pointerT* element)   const { if(!_interface) return false;
                                                        return _interface->writeData(element);}
    virtual bool addData(pointerT* element, keyT& key)     const { if(!_interface) return false;
                                                        return _interface->addData(element, key);}
    virtual bool deleteData(pointerT* element)  const { if(!_interface) return false;
                                                        return _interface->deleteData(element);}
    virtual pointerT* newElement()              const { if(!_interface) return 0;
                                                        return _interface->newElement();}
    virtual void cacheChanged()                 const {if(_interface) return _interface->cacheChanged();}
    
    virtual bool copy(const pointerT* from, pointerT* to) const { if(!_interface) return true;
                                                                    return _interface->copy(from, to);}
};

}   // namespace end: gmisc

#endif // SMARTCACHE_H
