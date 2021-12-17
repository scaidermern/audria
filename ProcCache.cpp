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
  userTimeJiffies(stringToNumber<uint64_t>(status[UserTimeJiffies])),
  systemTimeJiffies(stringToNumber<uint64_t>(status[SystemTimeJiffies])),
  startTimeJiffies(stringToNumber<uint64_t>(status[StartTimeJiffies])),
  runTimeSecs(0.0),
  totReadBytes(stringToNumber<uint64_t>(status[TotReadBytes])),
  totReadBytesStorage(stringToNumber<uint64_t>(status[TotReadBytesStorage])),
  totWrittenBytes(stringToNumber<uint64_t>(status[TotWrittenBytes])),
  totWrittenBytesStorage(stringToNumber<uint64_t>(status[TotWrittenBytesStorage])),
  totReadCalls(stringToNumber<uint64_t>(status[TotReadCalls])),
  totWriteCalls(stringToNumber<uint64_t>(status[TotWriteCalls])) {
}
