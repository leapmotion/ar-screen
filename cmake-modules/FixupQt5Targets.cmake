#fixup_qt5_targets
#makes Qt5::Gui depend on Qt5::QWindowsIntegrationPlugin on windows, and sets
#the INSTALL_SUBDIR property on QWindowsIntegrationPlugin so that target_imported_libraries
#will copy it to the correct location.  Must be called after qt5 is found
function(fixup_qt5_targets)
  set(_qt5_modules Core DBus Gui LinguistTools Multimedia MultimediaWidgets Network OpenGL Positioning PrintSupport Qml Quick QXcbIntegrationPlugin Script Sensors Sql WebKit WebKitWidgets Widgets)
  if(WIN32)
  	set_property(TARGET Qt5::QWindowsIntegrationPlugin PROPERTY INSTALL_SUBDIR platforms)
  	set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_MODULES Qt5::QWindowsIntegrationPlugin)

    # TODO: link libicu statically with Qt to remove three DLLs
    set(ICU_VERSION "52.1")
    set(ICU_DIR ${EXTERNAL_LIBRARY_DIR}/icu-${ICU_VERSION}${ALTERNATE_LIBRARY})
    foreach(icu_dll icudt52 icuuc52 icuin52)
    	add_library(ICU::${icu_dll} MODULE IMPORTED)
    	set_property(TARGET ICU::${icu_dll} PROPERTY IMPORTED_LOCATION ${ICU_DIR}/bin/${icu_dll}${SHARED_LIBRARY_SUFFIX})
    	set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_LINK_MODULES ICU::${icu_dll})
    endforeach()
  elseif(BUILD_LINUX AND NOT BUILD_ANDROID)
    set_property(TARGET Qt5::QXcbIntegrationPlugin PROPERTY INSTALL_SUBDIR platforms)
    set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_MODULES Qt5::QXcbIntegrationPlugin)

    #The QT targets only define imported_location for the release configuration, so we have to copy it over
    set(_qt5_modules_linux_so ${_qt5_modules} DBus QXcbIntegrationPlugin)
    list(REMOVE_ITEM _qt5_modules_linux_so LinguistTools)
    foreach(lib ${_qt5_modules_linux_so})
      get_target_property(imported_location Qt5::${lib} IMPORTED_LOCATION_RELEASE)
      set_property(TARGET Qt5::${lib} PROPERTY IMPORTED_LOCATION ${imported_location})
    endforeach()

  endif()

  foreach(lib ${_qt5_modules})
    mark_as_advanced(Qt5${lib}_DIR)
  endforeach()
endfunction()
