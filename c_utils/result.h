#ifndef _RESULT_H_
#define _RESULT_H_

/**
 * @brief Rust inspired Result type
 *
 * Create a new type using the DEFINE_RESULT macro
 *
 * T: type of the result value in case of no error
 * E: type of the error, for example an enduml
 * Name: name of the new result type
 */
#define DEFINE_RESULT(T, E, NAME) \
typedef struct { \
  bool success : 1; \
  union { \
    T result; \
    E error; \
  }; \
} NAME;

#endif