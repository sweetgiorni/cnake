
#include <iostream>
#include <stdexcept>

#include <python3.8/Python.h>

#include <boost/core/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/default_call_policies.hpp>
#include <boost/python/return_value_policy.hpp>

//#include "cmPolicies.h"
#include "cmake.h"

#include "CommandNames.h"

struct iterable_converter {
  /// @note Registers converter from a python interable type to the
  ///       provided type.
  template <typename Container> iterable_converter &from_python() {
    boost::python::converter::registry::push_back(
        &iterable_converter::convertible,
        &iterable_converter::construct<Container>,
        boost::python::type_id<Container>());

    // Support chaining.
    return *this;
  }

  /// @brief Check if PyObject is iterable.
  static void *convertible(PyObject *object) {
    return PyObject_GetIter(object) ? object : nullptr;
  }

  /// @brief Convert iterable PyObject to C++ container type.
  ///
  /// Container Concept requirements:
  ///
  ///   * Container::value_type is CopyConstructable.
  ///   * Container can be constructed and populated with two iterators.
  ///     I.e. Container(begin, end)
  template <typename Container>
  static void
  construct(PyObject *object,
            boost::python::converter::rvalue_from_python_stage1_data *data) {
    namespace python = boost::python;
    // Object is a borrowed reference, so create a handle indicting it is
    // borrowed for proper reference counting.
    python::handle<> handle(python::borrowed(object));

    // Obtain a handle to the memory block that the converter has allocated
    // for the C++ type.
    using storage_type =
        python::converter::rvalue_from_python_storage<Container>;
    void *storage = reinterpret_cast<storage_type *>(data)->storage.bytes;

    using iterator = python::stl_input_iterator<typename Container::value_type>;

    // Allocate the C++ type into the converter's memory block, and assign
    // its handle to the converter's convertible variable.  The C++
    // container is populated by passing the begin and end iterators of
    // the python object to the container's constructor.
    new (storage) Container(iterator(python::object(handle)), // begin
                            iterator());                      // end
    data->convertible = storage;
  }
};

#include "cmExecutionStatus.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmProjectCommand.h"
#include "cmState.h"

#undef BOOST_PYTHON_MAX_ARITY
#define BOOST_PYTHON_MAX_ARITY 16

class cmPyWrapper {
public:
  std::unique_ptr<cmake> CMakeInstance;
  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;

  cmPyWrapper(const std::vector<std::string> &args) {
    this->CMakeInstance =
        std::make_unique<cmake>(cmake::RoleProject, cmState::Project);

    this->CMakeInstance->SetArgs(args);
    if (cmSystemTools::GetErrorOccuredFlag()) {
      throw std::runtime_error("SetArgs failed");
    }
    if (this->CMakeInstance->LoadCache() < 0) {
      throw std::runtime_error("LoadCache failed");
    }
    CMakeInstance->ProcessPresetVariables();
    CMakeInstance->ProcessPresetEnvironment();
    if (!CMakeInstance->SetCacheArgs(args)) {
      cmSystemTools::Error("Run 'cmake --help' for all supported options.");
      std::runtime_error("SetCacheArgs failed");
    }
    if (cmSystemTools::GetErrorOccuredFlag()) {
      std::runtime_error("An unknown error occurred");
    }
    CMakeInstance->PreLoadCMakeFiles();
    CMakeInstance->GetCurrentSnapshot().SetDefaultDefinitions();
    // Makefile =
    //   std::make_unique<cmMakefile>(this->CMakeInstance->GetGlobalGenerator(),
    //                                this->CMakeInstance->GetCurrentSnapshot());

    // if (!Makefile) {
    //   throw std::runtime_error("Makefile is not initialized");
    // }

    int ret = this->CMakeInstance->Configure();
    if (ret) {
      throw std::runtime_error("Configure failed");
    }
  }

  void ExecuteCallback(std::vector<std::string> const &args,
                       const std::string &name) const {

    const std::unique_ptr<cmMakefile> &Makefile =
        this->CMakeInstance->GetGlobalGenerator()->GetMakefiles()[0];
    // if (!command(args, *ExecutionStatus)) {
    std::vector<cmListFileArgument> lffArgs;
    lffArgs.reserve(args.size());
    for (auto s : args) {
      // Assume all arguments are quoted.
      lffArgs.emplace_back(s, cmListFileArgument::Quoted, 0);
    }
    cmListFileFunction lff{name, 0, std::move(lffArgs)};
    cmExecutionStatus ExecutionStatus(*Makefile);
    if (!Makefile->ExecuteCommand(lff, ExecutionStatus)) {
      std::string const error = std::string(ExecutionStatus.GetError());
      throw std::runtime_error("Command failed: " + error);
    }
  }

  void Configure() const {
    if (this->CMakeInstance->Generate()) {
      throw std::runtime_error("Generate failed");
    }
  }
};

BOOST_PYTHON_MODULE(_cnake) {
  // Register interable conversions.
  iterable_converter().from_python<std::vector<std::string>>();

  auto wrapper =
      boost::python::class_<cmPyWrapper, boost::noncopyable>(
          "CMake", boost::python::init<const std::vector<std::string> &>())
          .def("configure", &cmPyWrapper::Configure);

  for (auto const &name : CommandNames) {
    auto fp = boost::python::make_function(
        [name](const cmPyWrapper &instance,
               const std::vector<std::string> &args) {
          return instance.ExecuteCallback(args, name);
        },
        boost::python::default_call_policies(),
        boost::mpl::vector<void, const cmPyWrapper &,
                           const std::vector<std::string> &>());
    wrapper.def(name, fp);
  }
}