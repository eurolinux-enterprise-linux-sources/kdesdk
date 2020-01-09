project(kbugbuster)
KDE4_NO_ENABLE_FINAL(kbugbuster)
add_definitions (-DQT3_SUPPORT_WARNINGS)

set(libkbbmainwindow_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/packagelvi.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/buglvi.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwloadingwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwsearchwidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwbugdetailscontainer.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwbuglistcontainer.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwbugdetails.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/centralwidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/kbbmainwindow.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/msginputdialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/packageselectdialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/messageeditor.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/severityselectdialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/preferencesdialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/loadallbugsdlg.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/serverconfigdialog.cpp
	)
kde4_add_ui_files(libkbbmainwindow_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/gui/centralwidget_base.ui
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwsearchwidget_base.ui
	${CMAKE_CURRENT_SOURCE_DIR}/gui/cwbugdetailscontainer_base.ui)
set(libkbbackend_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bug.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/backend/bugcache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugcommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugdetails.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugdetailsjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugjob.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/backend/buglistjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugmybugsjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugserverconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/bugsystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/domprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/htmlparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/kbbprefs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mailsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/package.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/packagelistjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/person.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/rdfprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/smtp.cpp
	)

add_subdirectory( gui ) 
add_subdirectory( pics ) 
add_subdirectory(kresources)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/backend  )


########### next target ###############

set(kbugbuster_SRCS main.cpp ${libkbbmainwindow_SRCS} ${libkbbackend_SRCS})

kde4_add_app_icon(kbugbuster_SRCS "${KDE4_INSTALL_DIR}/share/icons/oxygen/*/apps/kbugbuster.png")

kde4_add_executable(kbugbuster ${kbugbuster_SRCS})

target_link_libraries(kbugbuster  ${KDE4_KUTILS_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kbugbuster  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kbugbuster.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

