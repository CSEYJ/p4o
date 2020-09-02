#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.



set (P4O_SRCS
  backend.cpp
  main.cpp
  dataDependencyAnalysis.cpp
  extractTableCondition.cpp
  ../bmv2/psa_switch/midend.cpp
  ../bmv2/common/lower.cpp
  ../bmv2/simple_switch/simpleSwitch.cpp
  ../bmv2/common/JsonObjects.cpp
  ../bmv2/common/action.cpp
  ../bmv2/common/controlFlowGraph.cpp
  ../bmv2/common/deparser.cpp
  ../bmv2/common/expression.cpp
  ../bmv2/common/extern.cpp
  ../bmv2/common/globals.cpp
  ../bmv2/common/header.cpp
  ../bmv2/common/helpers.cpp
  ../bmv2/common/lower.cpp
  ../bmv2/common/metermap.cpp
  ../bmv2/common/parser.cpp
  ../bmv2/common/programStructure.cpp
  )

set (P4O_HDRS
  backend.h
  ../bmv2/psa_switch/midend.h
  )

add_cpplint_files(${CMAKE_CURRENT_SOURCE_DIR} "${P4O_SRCS};${P4O_HDRS}")
build_unified(P4OSRCS ALL)

add_executable(p4o ${P4O_SRCS})
target_link_libraries (p4o ${P4C_LIBRARIES} ${P4C_LIB_DEPS})
add_dependencies(p4o genIR frontend)

install(TARGETS p4o RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
add_custom_target(linkp4o
  COMMAND ${CMAKE_COMMAND} -E create_symlink `realpath --relative-to=${P4C_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/p4o` ${P4C_BINARY_DIR}/p4o
  )
add_dependencies(p4o linkp4o)