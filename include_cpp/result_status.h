#if !defined(RESULT_STATUS_H)
#define RESULT_STATUS_H

// values returned from Task Processor / Preparer section
// to the caller

#define MSB_RESULT_ERROR  -1
#define MSB_RESULT_GOOD    0
#define MSB_RESULT_FAIL    1
#define MSB_RESULT_BUSY    2 // only between client and server

#endif // !defined(RESULT_STATUS_H)