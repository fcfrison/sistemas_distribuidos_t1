#include <stdlib.h>
#include "../include/simple_map.h"

KeyValuePair*
create_key_val_pair(void* key, void* value){
    if(!key) return NULL;
    KeyValuePair* kvp = (KeyValuePair*)calloc(1,sizeof(KeyValuePair));
    kvp->key   = key;
    kvp->value = value;
    return kvp;
}

int
remove_key(SimpleMap* sm,
           const void* key,
           const void*(cmp_fptr)(const void* a, const void* b),
           KeyValuePair* rmv_pair){
    if(!sm       || !key || !cmp_fptr || !rmv_pair ||
       !sm->keys || !sm->values){
        return REMOVE_ERROR;
    }
    int i = 0;
    void* rmv_key = NULL, *rmv_value = NULL; 
    while(i<=sm->top){
        if(!sm->keys[i]){
            return REMOVE_ERROR;
        }
        rmv_key = sm->keys[i]->key;
        if(!cmp_fptr(key,rmv_key)){
            i++;
            continue;
        };
        rmv_value = sm->values[i]->value;
        while(1){
            if(i==sm->top){
                sm->keys[i] = NULL;
                sm->values[i] = NULL;
                sm->top--;
                rmv_pair->key   = rmv_key;
                rmv_pair->value = rmv_value;
                return REMOVE_SUCCESS;
            }
            sm->keys[i] = sm->keys[i+1];
            sm->values[i] = sm->values[i+1];
            i++;
        }
    }
    return REMOVE_KEY_NOT_FOUND;
}
unsigned char __is_full(SimpleMap* sm){
    if(sm->top+1==sm->capacity){
        return 1;
    }
    return 0;
}
SimpleMap*
__double_arrays(SimpleMap* sm){
    if(!sm || !sm->keys || !sm->values){
        return NULL;
    }
    if(!sm->capacity || !__is_full(sm)){
        return NULL;
    }
    KeyWrapper**   keys   = NULL;
    ValueWrapper** values = NULL;
    size_t new_capacity = 2 * sm->capacity;
    keys     = (KeyWrapper**)realloc(sm->keys,new_capacity*sizeof(KeyWrapper*));
    if(!keys){
        return NULL;
    }
    values   = (ValueWrapper**)realloc(sm->values,new_capacity*sizeof(ValueWrapper*));
    if(!values){
        sm->keys = realloc(keys, sm->capacity * sizeof(KeyWrapper*));
        return NULL;
    }
    sm->keys   = keys;
    sm->values = values;
    sm->capacity = new_capacity;
    return sm;
}

KeyValuePair*
__set(SimpleMap* sm, KeyValuePair* key_par){
    if(!sm           || !key_par    || 
       !sm->keys     || !sm->values || 
       !key_par->key || !sm->capacity){
        return NULL;
    }
    KeyWrapper*   key   = (KeyWrapper*)calloc(1,sizeof(KeyWrapper));
    ValueWrapper* value = (ValueWrapper*)calloc(1,sizeof(ValueWrapper));
    key->key            = key_par->key;
    value->value        = key_par->value;
    if(__is_full(sm)){
        if(!__double_arrays(sm)){
            free(key);
            free(value);
            return NULL;
        }
    }
    sm->top++;
    sm->keys[sm->top]   = key;
    sm->values[sm->top] = value;
    return key_par;
}
KeyValuePair*
__upgrade(SimpleMap* sm, int pos, KeyValuePair* key_par){
    if(!sm       || !key_par || !sm->values   ||
       !sm->keys || pos<0    || pos > sm->top ||
       !key_par->key){
        return NULL;
    }
    void* old_v = sm->values[pos]->value;
    void* old_k = sm->keys[pos]->key;
    sm->values[pos]->value = key_par->value;
    sm->keys[pos]->key     = key_par->key;
    key_par->key           = old_k;
    key_par->value         = old_v;
    return key_par;
}



KeyValuePair*
get(SimpleMap* sm, void* key, const void*(cmp_fptr)(const void* a, const void* b)){
    if(!sm || !key || !cmp_fptr ){
        return NULL;
    }
    int pos = __find(sm, key, cmp_fptr);
    switch (pos){
        case KEY_NOT_FOUND:
        case FIND_KEY_ERROR:
        case KEY_ARR_ERROR:
            return NULL;
        default:
            KeyValuePair* kvp = (KeyValuePair*)calloc(1,sizeof(KeyValuePair));
            if(!kvp) return NULL;
            kvp->key   = sm->keys[pos]->key;
            kvp->value = sm->values[pos]->value; 
            return kvp;
    }
}
int
__find(const SimpleMap* sm, const void* key, const void*(cmp_fptr)(const void* a, const void* b)){
    int i = 0;
    if(!sm || !key || !cmp_fptr ||
       !sm->keys){
        return FIND_KEY_ERROR;
       }
    while(i <= sm->top){
        if(!sm->keys[i]){
            return KEY_ARR_ERROR;
        }
        const void* srch_el = cmp_fptr(sm->keys[i]->key,key);
        if(srch_el){
            return i;
        }
        i++;    
    }
    return KEY_NOT_FOUND;
}
SimpleMap*
create_simple_map(void){
    SimpleMap* sm = (SimpleMap*)calloc(1,sizeof(SimpleMap));
    if(!sm){
        return NULL;
    }
    KeyWrapper** k_arr  = (KeyWrapper**)calloc(1,sizeof(KeyWrapper*));
    if(!k_arr){
        free(sm);
        return NULL;
    }
    ValueWrapper** v_arr = (ValueWrapper**)calloc(1,sizeof(ValueWrapper*));
    if(!v_arr){
        free(sm);
        free(k_arr);
        return NULL;
    }
    sm->capacity =  1;
    sm->top      = -1;
    sm->keys     = k_arr;
    sm->values   = v_arr;
    return sm;
};

int
delete_map(SimpleMap* sm, void* (*clean_up)(void*, void*)){
    if(!sm || !sm->keys || !sm->values ){
        return 0;
    }
    int i = 0;
    while(i <= sm->top){
        if(clean_up && sm->keys[i] && sm->values[i]){
            clean_up(sm->keys[i]->key,sm->values[i]->value);
        }
        if(sm->keys[i]){
            free(sm->keys[i]);
        }
        if(sm->values[i]){
            free(sm->values[i]);
        }
        i++;
    }
    free(sm->keys);
    free(sm->values);
    free(sm);
    return 1;
};
int
set(SimpleMap* sm, KeyValuePair* key_par, const void*(cmp_fptr)(const void* a, const void* b)){
    if(!sm || !key_par || !key_par->key){
        return ERROR_SET_SM_RTN;
    }
    int pos = __find(sm, key_par->key,cmp_fptr);
    switch (pos){
        case FIND_KEY_ERROR:
        case KEY_ARR_ERROR:
            return ERROR_SET_SM_RTN;
        case KEY_NOT_FOUND:
            if(!__set(sm, key_par)){
                return ERROR_SET_SM_RTN;
            }
            return SUCESS_SET;
        default:
            if(!__upgrade(sm, pos, key_par)){
                return ERROR_SET_SM_RTN;
            }
            return SUCESS_UPGRADE;
    };
};