cmake_minimum_required(VERSION 3.16)
project(MFC LANGUAGES CXX)

set(ROWAN_MFC_SOURCES
    uid_reverse.cpp
    MFC_stub.cpp
    MIGVIEW.CPP
    CntrItem.cpp
    CREDITS.CPP
    Fullpane.cpp
    ../Input/keystub.cpp
    MAINFRM.CPP
    Maintbar.cpp
    MapDlg.cpp
    MIG.cpp
    MIGDoc.cpp
    RDEmptyP.cpp
    RDEmptyD.cpp
    RDialog.cpp
    STUB3D.CPP
    Hintbox.cpp
    Listbx.cpp
	sysbox.cpp
	Msctlbr.cpp
	Titlebar.cpp
	thumnail.cpp
    RDIALLOG.CPP
)

set(ROWAN_AFX_SOURCES
    FONT.CPP
    PICTURE.CPP
    RBUTTON.CPP
    RCOMBO.CPP
    RCOMBOX.CPP
    REDIT.CPP
    REDTBT.CPP
    RJOYCFG.CPP
    RLISTBOX.CPP
    RSPLTBAR.CPP
    RTABS.CPP
    RTITLE.CPP
    RSCRLBAR.CPP
    RSPINBUT.CPP
)

set(ROWAN_SHEETS_SOURCES
    AC_VIEW.CPP
    APILOT.CPP
    ARMY.CPP
    ARMYDETL.CPP
    ARMYRQST.CPP
    ARMY_DOS.CPP
    AUTHOR.CPP
    BASES.CPP
    BASEST.CPP
    BATTDETL.CPP
    CAMPBACK.CPP
    CAMPLIST.CPP
    Campwar.cpp
    Career.cpp
    CHOKE.CPP
    CHSQUAD.CPP
    CMBTVIEW.CPP
    COMIT_E.CPP
    COMMSPNT.CPP
    COMMSAC.CPP
    CURREMBL.CPP
    DAMAGE.CPP
    DBRFTLBR.CPP
    DIRECTS2.CPP
    DIS.CPP
    DIS_NOTE.CPP
    DOSBUT.CPP
    DOSSIER.CPP
    FILING.CPP
    FLT_TASK.CPP
    FOOTAGE.CPP
    FRAG.CPP
    FRAGPILT.CPP
    GENERAL.CPP
    GRNDVIEW.CPP
    io_compat.cpp
    LOAD.CPP
    LOADPROF.CPP
    LOCKER.CPP
    LSTMSNLG.CPP
    MAPFLTRS.CPP
    MISNTEXT.CPP
    MISN_LOG.CPP
    MISSFLDR.CPP
    MSNDEBRF.CPP
    MRESULT.CPP
    MTCHILD.CPP
    NOSE_ART.CPP
    NOTES.CPP
    OVERVIEW.CPP
    PAINT.CPP
    PERSONAL.CPP
    PILOTS.CPP
    PLYR_LOG.CPP
    PROFILE.CPP
    PROF_LIB.CPP
    RADIO.CPP
    Ready.cpp
    RIP_LIST.CPP
    RMDLDLG.CPP
    ROUTE.CPP
    RowTasks.cpp
    RRADIO.CPP
    RSTATIC.CPP
    RTESTSH1.CPP
    RTOOLBAR.CPP
    SAIRCLMS.CPP
    SAVEPROF.CPP
    SCALEBAR.CPP
    SCAMP.CPP
    Scontrol.cpp
    SDETAIL.CPP
    SELECTS.CPP
    SERVICE.CPP
    SESSION.CPP
    SFLIGHT.CPP
    SGAME.CPP
    SGRDCLMS.CPP
    SMKDLG.CPP
    SQDNLB.CPP
    SQDNLIST.CPP
    SQUADS.CPP
    SQUICK1.CPP
    SQDNLBUT.CPP
    SQUICK2.CPP
    SQUICKP.CPP
    SQUICKUN.CPP
    SQVIEW.CPP
    SREPLAY.CPP
    SSOUND.CPP
    SUPPLY.CPP
    SVIEWER.CPP
    TASK.CPP
    TASKTABS.CPP
    USERINST.CPP
    VARIANTS.CPP
    VISITORS.CPP
    VWQLIST.CPP
    WAVEINS.CPP
    WAVETABS.CPP
    WIN32_COMPAT.CPP
    WEAPONS.CPP
    WEATHER.CPP
    WPBUT.CPP
    WPDETAIL.CPP
    WPGROUND.CPP
    WPSHARE.CPP

)

# ---------------------------------------------------------
# Create the three static libraries
# ---------------------------------------------------------

add_library(RowanMFC STATIC ${ROWAN_MFC_SOURCES})
add_library(RowanAFX STATIC ${ROWAN_AFX_SOURCES})
add_library(RowanSheets STATIC ${ROWAN_SHEETS_SOURCES})


foreach(target RowanMFC RowanAFX RowanSheets)
    # Inherit common includes/defines
    target_link_libraries(${target} PRIVATE ProjectDefaults)

    # Extra include dirs specific to UI
    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            #${CMAKE_CURRENT_SOURCE_DIR}/../MigAlley
    )
endforeach()
