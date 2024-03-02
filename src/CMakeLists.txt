set(WYZEDOM_SOURCE
  musl_shim.c
  main.cpp
  IMP.cpp
  Encoder.cpp
  Logger.cpp
  RTSP.cpp
  IMPServerMediaSubsession.cpp
  IMPDeviceSource.cpp
  GPIO.cpp
  Scripts.cpp
  Config.cpp
)

add_executable(${PROJECT_NAME} ${WYZEDOM_SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC
  ${LIVE555_INCLUDE_DIR}
  ${LIVE555_INCLUDE_DIR}/liveMedia
  ${LIVE555_INCLUDE_DIR}/groupsock
  ${LIVE555_INCLUDE_DIR}/UsageEnvironment
  ${LIVE555_INCLUDE_DIR}/BasicUsageEnvironment
  ../blob/include
)

target_link_directories(${PROJECT_NAME} PUBLIC
  ${LIVE555_LIB_DIR}
)

target_link_libraries(${PROJECT_NAME}
  pthread
  imp alog rt
  liveMedia
  groupsock
  BasicUsageEnvironment
  UsageEnvironment
  config++
)
