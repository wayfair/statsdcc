if (test)
  add_executable(ledger_test ledger_test.cc)
  target_link_libraries(ledger_test
    ledger
    backend_config
    config
    logger
    ${JSONCPP}
    gtest
    gtest_main
  )

  add_executable(hashring_test hashring_test.cc)
  target_link_libraries(hashring_test
    hashring
    crypto
    gtest
    gtest_main
  )

  configure_file(data/nodejs_hashring.data data/nodejs_hashring.data COPYONLY)
  configure_file(data/nodejs_hashring_get.data data/nodejs_hashring_get.data COPYONLY)
endif()
