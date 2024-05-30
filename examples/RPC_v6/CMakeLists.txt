add_executable(Weight Weight.C)
target_link_libraries(Weight RPCSim::RPCSim)
install(TARGETS Weight
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas" DESTINATION "${DATA_FOLDER}/examples/")
