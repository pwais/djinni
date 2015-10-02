test:
 * djinni interface that has methods that do stuff
 * junit tests that exec above interface
 * test pass null heap array and shows null on cpp
 * test create null heap array on cpp and nothing happens, shows as null on java side
 * test create non-empty heap array, set bytes, verify in cpp side
 * test create non-empty heap array in cpp, set bytes, verify on java side
 * do above tests for direct byte buffer
