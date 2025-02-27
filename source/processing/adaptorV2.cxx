/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "processing/adaptorV2.h"
#include "utils/logger.h"

#include <iostream>
#include <math.h>

#include <vtkDataObject.h>
#include <vtkNew.h>
namespace fs = boost::filesystem;

Processor::Processor( vtkMPICommunicatorOpaqueComm& communicator
                    , const Configuration &config)
{
  BOOST_LOG_TRIVIAL(trace) << "Starting Catalyst2 ...";

  auto scripts = config.collectScripts();
  for (const fs::path& script : scripts)
  {
    const auto path = std::string(script.c_str());
    const auto script_name =  path.substr(path.find_last_of("/\\") + 1);
    // note: one can simply add the script file as follows:
    //node["catalyst/scripts/script" + settings.catalyst_script].set_string(path);

    // alternatively, use this form to pass optional parameters to the script.
    const auto name = "catalyst/scripts/script_" + script_name;
    node[name + "/filename"].set_string(path);
    node[name + "/args"].append().set_string("--channel-name=mainGrid");

    // add the name of all variables to the args so that scripts can use them to read each variable needed
    std::vector<std::string> vars = config.collectVariables();
    for (int numVars = 0; numVars < vars.size(); numVars++)
    {
      node[name + "/args"].append().set_string("--variable-" + std::to_string(numVars) + "=" + vars[numVars]);
    }

    // indicate that we want to load ParaView-Catalyst
    node["catalyst_load/implementation"].set_string("paraview");
    
    // Use this for debugging. Set environment variable CATALYST_DATA_DUMP_DIRECTORY=dir
    //node["catalyst_load/implementation"].set_string("stub");
    node["catalyst_load/search_paths/paraview"].set_string(std::string(config.getCatalystLib().c_str()));
    
    // set the communicator that we have been given
    node["catalyst/mpi_comm"].set(MPI_Comm_c2f(*(communicator.GetHandle())));

    BOOST_LOG_TRIVIAL(trace) << "Initializing catalyst with:---- ";
    BOOST_LOG_TRIVIAL(trace) << node.to_string();

    catalyst_status err = catalyst_initialize(conduit_cpp::c_node(&node));
    if (err != catalyst_status_ok)
    {
        BOOST_LOG_TRIVIAL(trace) << "Failed to initialize Catalyst: " << err;
    }
  }

  BOOST_LOG_TRIVIAL(trace) << "\t\t...Done";
  BOOST_LOG_TRIVIAL(info) << "Please connect in Paraview";

  auto delay = config.getStartupDelay();
  sleep(delay);
}

Processor::~Processor()
{
  catalyst_status err = catalyst_finalize(conduit_cpp::c_node(&node));
  if (err != catalyst_status_ok)
  {
    BOOST_LOG_TRIVIAL(trace) << "ERROR: Failed to finalize Catalyst: " << err;
  }
  else
  {
    BOOST_LOG_TRIVIAL(trace) << "FINALIZED Catalyst success.";
  }
}

/*
 * Descriptor is a term from catalyst 1. Used to store information that gets passed between the adaptor and the pipelines.
 *
 * With catalyst2, we store this kind of information in a conduit node 
*/
Descriptor::Descriptor( Processor& processor_
                      , const MPIInportSection& section_
                      , uint timeStep, double time, bool forceOutput)
  : node(processor_)
  , requireProcessing(true)
  , section(section_)
{
  auto state = description["catalyst/state"];
  state["timestep"].set(timeStep);
  state["time"].set(time);
}

Descriptor::~Descriptor()
{
  if (requireProcessing)
  {
    description.print();
    catalyst_status err = catalyst_execute(conduit_cpp::c_node(&description));
    if (err != catalyst_status_ok)
    {
        BOOST_LOG_TRIVIAL(trace) << "Failed to execute Catalyst: " << err;
    }
  }
}

bool Descriptor::doesRequireProcessing() const
{
  return requireProcessing;
}


Adaptor::Adaptor( Descriptor& descriptor_
                , const std::string& name_)
  : descriptor(descriptor_)
  , name(name_)
{
  // addInput seems analgous to a channel
  //auto channel = descriptor.description["catalyst/channels/" + name];
  auto channel = descriptor.description["catalyst/channels/mainGrid"];
}

Adaptor::~Adaptor()
{
}

bool Adaptor::doesRequireProcessing() const
{
  return descriptor.doesRequireProcessing();
}

Adaptor::Extent Adaptor::getExtent(size_t max) const
{
  const MPIInportSection& section(descriptor.getSection());
  size_t chunksize = static_cast<size_t>(
                       ceil(static_cast<double>(max) /
                            static_cast<double>(section.getSize())));
  size_t extstart = std::min(chunksize * section.getIndex(), max);
  size_t extsize = std::min(chunksize, max - extstart);

  return Extent(extstart, extsize);
}

const MPIInportSection& Adaptor::getSection() const
{
  return descriptor.getSection();
}


void Adaptor::coprocess(vtkDataObject* data, int global_extent[6])
{
  //Check if the chanel we need actually exists
  //if(descriptor.description.has_path("catalyst/channels/" + name))
  if(descriptor.description.has_path("catalyst/channels/mainGrid"))
  {
    //conduit_cpp::Node channel = descriptor.description["catalyst/channels/" + name];
    conduit_cpp::Node channel = descriptor.description["catalyst/channels/mainGrid"];
    channel["type"].set("mesh");
    // now create the mesh.
    conduit_cpp::Node mesh = channel["data"];
    
    // set the vtkobject to a local varaible so that it is not lost when this class is destructed (ensuring 0 copy)
    descriptor.descriptor_data = data;
    
    bool is_success =
      vtkDataObjectToConduit::FillConduitNode(vtkDataObject::SafeDownCast(descriptor.descriptor_data), mesh);
      
    if (!is_success)
    {
      BOOST_LOG_TRIVIAL(trace) << "FillConduitNode failed for adaptorV2";
    } 
  }
}
