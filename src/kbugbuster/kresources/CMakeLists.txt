
include_directories( ../backend  )


########### next target ###############

set(kcal_bugzilla_PART_SRCS kcalresource.cpp kcalresourceconfig.cpp kcalresource_plugin.cpp ${libkbbackend_SRCS})


kde4_add_kcfg_files(kcal_bugzilla_PART_SRCS resourceprefs.kcfgc )

kde4_add_plugin(kcal_bugzilla ${kcal_bugzilla_PART_SRCS})



target_link_libraries(kcal_bugzilla  ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KCAL_LIBS}
    ${KDE4_KRESOURCES_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcal_bugzilla  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES bugzilla.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kresources/kcal )

