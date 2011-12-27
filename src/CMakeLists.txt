
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kredentials_SRCS main.cpp kredentials.cpp krb5_wrap.cpp)

kde4_add_executable(kredentials ${kredentials_SRCS})

target_link_libraries(kredentials ${QT_QTGUI_LIBRARY} QtGui)
target_link_libraries(kredentials ${KDE4_KDECORE_LIBS} kdeui)
target_link_libraries(kredentials ${KRB5_LIBRARIES} krb5)

install(TARGETS kredentials ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kredentials.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})
