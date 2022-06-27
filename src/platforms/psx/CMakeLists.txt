cmake_minimum_required(VERSION 3.18.0)


# project(
# 	TotalSMS_PSX
# 	LANGUAGES C
# )

psn00bsdk_add_executable(TotalSMS_PSX STATIC main.c)
target_link_libraries(TotalSMS_PSX TotalSMS)
install(FILES ${PROJECT_BINARY_DIR}/TotalSMS_PSX.exe TYPE BIN)

# psn00bsdk_add_cd_image(
# 	iso      # Target name
# 	template # Output file name (= TotalSMS_PSX.bin + TotalSMS_PSX.cue)
# 	iso.xml  # Path to config file
# 	DEPENDS TotalSMS_PSX
# )

#psn00bsdk_add_cd_image(TotalSMS_PSX_iso TotalSMS_PSX iso.xml DEPENDS TotalSMS_PSX)
