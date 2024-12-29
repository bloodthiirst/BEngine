CALL rmdir /s /q "CMakeGenerated"
CALL rmdir /s /q "Build"
CALL rmdir /s /q "lib"
CALL vcvarsall x64
CALL cmake -B ./CMakeGenerated
CALL cmake --build ./CMakeGenerated
pause