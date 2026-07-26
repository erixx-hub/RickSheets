cmake_minimum_required(VERSION 3.24)
project(RickSheets VERSION 0.3.2 LANGUAGES CXX)

include(GNUInstallDirs)

set(RICKSHEETS_APP_ID "io.github.erixx_hub.RickSheets")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Widgets PrintSupport Test LinguistTools)

add_library(ricksheets_core
    src/song.cpp
    src/song.h
    src/chordparser.cpp
    src/chordparser.h
    src/renderer.cpp
    src/renderer.h
)
target_link_libraries(ricksheets_core PUBLIC Qt6::Core Qt6::Gui Qt6::PrintSupport)
target_include_directories(ricksheets_core PUBLIC src)

add_executable(ricksheets
    src/main.cpp
    src/language.cpp
    src/language.h
    src/appstyle.cpp
    src/appstyle.h
    src/blockeditordialog.cpp
    src/blockeditordialog.h
    src/chordeditordialog.cpp
    src/chordeditordialog.h
    src/importreviewdialog.cpp
    src/importreviewdialog.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/previewwidget.cpp
    src/previewwidget.h
)
target_link_libraries(ricksheets PRIVATE ricksheets_core Qt6::Widgets)
target_compile_definitions(ricksheets PRIVATE
    RICKSHEETS_VERSION="${PROJECT_VERSION}")

qt_add_translations(ricksheets
    SOURCE_TARGETS ricksheets ricksheets_core
    TS_FILES translations/ricksheets_en.ts
    LUPDATE_OPTIONS -no-obsolete
    RESOURCE_PREFIX "/i18n"
)

set(RICKSHEETS_BRAND_ASSETS
    ${CMAKE_SOURCE_DIR}/docs/Logo/ricksheets-favicon.svg
    ${CMAKE_SOURCE_DIR}/docs/Logo/ricksheets-icon-dark.svg
    ${CMAKE_SOURCE_DIR}/docs/Logo/ricksheets-icon-light.svg
    ${CMAKE_SOURCE_DIR}/docs/Logo/ricksheets-wordmark.svg
)

function(ricksheets_add_brand_resources target)
    qt_add_resources(${target} "ricksheets_brand"
        PREFIX "/brand"
        BASE ${CMAKE_SOURCE_DIR}/docs/Logo
        FILES ${RICKSHEETS_BRAND_ASSETS}
    )
    qt_add_resources(${target} "ricksheets_png_icon"
        PREFIX "/brand"
        BASE ${CMAKE_SOURCE_DIR}/packaging/linux/icons/hicolor/256x256/apps
        FILES
            ${CMAKE_SOURCE_DIR}/packaging/linux/icons/hicolor/256x256/apps/de.rickrich.RickSheets.png
    )
endfunction()

ricksheets_add_brand_resources(ricksheets)

enable_testing()
add_executable(ricksheets_tests tests/test_core.cpp src/previewwidget.cpp src/previewwidget.h)
target_link_libraries(ricksheets_tests PRIVATE ricksheets_core Qt6::Test Qt6::Widgets)
ricksheets_add_brand_resources(ricksheets_tests)
target_compile_definitions(ricksheets_tests PRIVATE
    RICKSHEETS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME core COMMAND ricksheets_tests)
set_tests_properties(core PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(ricksheets_ui_test
    tests/test_ui.cpp
    src/language.cpp
    src/language.h
    src/appstyle.cpp
    src/appstyle.h
    src/blockeditordialog.cpp
    src/blockeditordialog.h
    src/chordeditordialog.cpp
    src/chordeditordialog.h
    src/importreviewdialog.cpp
    src/importreviewdialog.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/previewwidget.cpp
    src/previewwidget.h
)
target_link_libraries(ricksheets_ui_test PRIVATE ricksheets_core Qt6::Test Qt6::Widgets)
ricksheets_add_brand_resources(ricksheets_ui_test)
add_test(NAME ui_smoke COMMAND ricksheets_ui_test)
set_tests_properties(ui_smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

find_program(PDFTOTEXT_EXECUTABLE pdftotext)
if(PDFTOTEXT_EXECUTABLE)
    add_executable(ricksheets_import_audit tests/import_audit.cpp)
    target_link_libraries(ricksheets_import_audit PRIVATE ricksheets_core Qt6::Gui)
    add_test(NAME template_import
             COMMAND ricksheets_import_audit ${CMAKE_SOURCE_DIR}/docs)
    set_tests_properties(template_import PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

install(TARGETS ricksheets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/linux/${RICKSHEETS_APP_ID}.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/linux/${RICKSHEETS_APP_ID}.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES docs/Logo/ricksheets-icon-dark.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME ${RICKSHEETS_APP_ID}.svg)
foreach(icon_size 48 64 128 256)
    install(FILES
        packaging/linux/icons/hicolor/${icon_size}x${icon_size}/apps/de.rickrich.RickSheets.png
        DESTINATION
        ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${icon_size}x${icon_size}/apps
        RENAME ${RICKSHEETS_APP_ID}.png)
endforeach()
install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${RICKSHEETS_APP_ID})
