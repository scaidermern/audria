#include "ProcCache.h"
#include "ProcReader.h"
#include "helper.h"

Cache::Cache() : isEmpty(true),
  userTimeJiffies(0), systemTimeJiffies(0), startTimeJiffies(0), runTimeSecs(0.0),
  totReadBytes(0), totReadBytesStorage(0), totWrittenBytes(0), totWrittenBytesStorage(0),
  totReadCalls(0), totWriteCalls(0) {
}

Cache::Cache(const ProcessStatus& status) :
  isEmpty(false),
  userTimeJiffies(stringToNumber<unsigned long long>(status[UserTimeJiffies])),
  systemTimeJiffies(stringToNumber<unsigned long long>(status[SystemTimeJiffies])),
  startTimeJiffies(stringToNumber<unsigned long long>(status[StartTimeJiffies])),
  runTimeSecs(0.0),
  totReadBytes(stringToNumber<unsigned long long>(status[TotReadBytes])),
  totReadBytesStorage(stringToNumber<unsigned long long>(status[TotReadBytesStorage])),
  totWrittenBytes(stringToNumber<unsigned long long>(status[TotWrittenBytes])),
  totWrittenBytesStorage(stringToNumber<unsigned long long>(status[TotWrittenBytesStorage])),
  totReadCalls(stringToNumber<unsigned long long>(status[TotReadCalls])),
  totWriteCalls(stringToNumber<unsigned long long>(status[TotWriteCalls])) {
}
