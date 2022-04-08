idf_component_register(SRCS "Display.cpp" 
                            "ScreenManager.cpp" 
                            "Screen.cpp" 
                            "ListScreen.cpp"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES tft chip)
