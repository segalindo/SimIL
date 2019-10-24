#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <contra/relay.hpp>
#include <contra/zmq/zeromq_transport.hpp>
#include <nesci/producer/spike_detector.hpp>
#include <simil/simil.h>
#ifdef SIMIL_USE_BRION
#include <simil/loaders/LoadblueConfigData.h>
#endif
#include <simil/loaders/LoadHDF5Data.h>

#include <chrono>
#include <thread>

// struct Spike {
//   double time;
//   unsigned int neuron_id;
// };

int main( int argc, char** argv )
{
  if ( argc < 2 )
  {
    std::cerr << "USAGE: loadRefactorExample -bc|-h5 file [target]"
              << std::endl;
#ifdef SIMIL_USE_BRION
    std::cerr << "-bc blueconfig loader" << std::endl;
#else
    std::cerr << "-bc NOT available blueconfig loader"
                 ", consider compile again with BRION support"
              << std::endl;
#endif
    std::cerr << "-h5 HDF5 loader " << std::endl;
    return 1;
  }
  std::string simtype = argv[ 1 ];
  std::string path = argv[ 2 ];
  std::string secondaryPath;

  simil::LoadSimData* importer;

#ifdef SIMIL_USE_BRION
  if ( simtype == "-bc" )
  {
    importer = new simil::LoadblueConfigData( );
  }
  else
#endif
    if ( simtype == "-h5" )
  {
    if ( argc < 4 )
    {
      std::cerr << "Error: an activity file must be provided after network file"
                << std::endl;
      return 1;
    }
    importer = new simil::LoadHDF5Data( );

    secondaryPath = argv[ 3 ];
  }
  else
  {
    importer = new simil::LoadHDF5Data( );
    std::cerr << "USAGE: loadRefactorExample -bc|-h5 file [target]"
              << std::endl;
#ifdef SIMIL_USE_BRION
    std::cerr << "-bc blueconfig loader" << std::endl;
#else
    std::cerr << "-bc NOT available blueconfig loader"
                 ", consider compile again with BRION support"
              << std::endl;
#endif

    std::cerr << "-h5 HDF5 loader " << std::endl;
    return 1;
  }

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Network" << std::endl;
  std::cout << "--------------------------------------" << std::endl;

  simil::SimulationData* simData =
    importer->LoadSimulationData( path, secondaryPath );

  std::cout << "Loaded GIDS: " << simData->gids( ).size( ) << std::endl;
  std::cout << "Loaded positions: " << simData->positions( ).size( )
            << std::endl;

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Spikes" << std::endl;
  std::cout << "--------------------------------------" << std::endl;

  simil::SpikeData* spkData = dynamic_cast< simil::SpikeData* >( simData );

  if ( spkData == nullptr )
  {
    std::cerr << "Error: this example is only prepared to spike data"
              << std::endl;
    return 1;
  }

  simil::TGIDSet gids = spkData->gids( );

  std::cout << "Loaded GIDS: " << gids.size( ) << std::endl;

  simil::TPosVect positions = spkData->positions( );

  std::cout << "Loaded positions: " << positions.size( ) << std::endl;

  simil::TSpikes spikes = spkData->spikes( );
  float startTime = spkData->startTime( );
  float endTime = spkData->endTime( );

  std::cout << "Loaded spikes: " << spikes.size( ) << std::endl;
  std::cout << "Starting from " << startTime << " to " << endTime << std::endl;

  std::cout << "--------------------------------------" << std::endl;


    contra::Relay<contra::ZMQTransport> relay(8000);

     nesci::producer::SpikeDetector sd{"spike_detector"};



     uint recorded_spikes = 0;

     do {

         for (int i=0; i < 10;i++) {
           sd.Record(spikes[recorded_spikes].first,
                     spikes[recorded_spikes].second);
           recorded_spikes++;
         }

         std::cout << "Sending! number:" <<recorded_spikes << std::endl;
           relay.Send(sd.node());
         std::this_thread::sleep_for(std::chrono::milliseconds(3000));
         sd.Clear();
     } while (recorded_spikes < spikes.size( ));


}
