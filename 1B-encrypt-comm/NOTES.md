# Socket Programming

## bzero
* DONT USE BZERO
* use memset or memcpy

## Encryption
* init the mcrypt and then mcrypt_generic_init()
* call mcrypt_module_open() to get the IV. Cannot use the same IV for both encryption and decryption
* use same key and same initialize

```c
// to get a truly random bunch of bytes
// use this to generate the seed
getrandom()
```
