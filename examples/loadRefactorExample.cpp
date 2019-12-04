/*
 * @file	simData.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include <simil/simil.h>
#ifdef SIMIL_USE_BRION
#include <simil/loaders/LoadBlueConfigData.h>
#endif
#include <simil/loaders/LoadHDF5Data.h>
#include <simil/loaders/LoadCSVData.h>
#include <iostream>

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
    std::cerr << "-csv CSV loader " << std::endl;
    return 1;
  }
  std::string simtype = argv[ 1 ];
  std::string path = argv[ 2 ];
  std::string secondaryPath;

  simil::LoadSimData* importer;

#ifdef SIMIL_USE_BRION
  if ( simtype == "-bc" )
  {
    importer = new simil::LoadBlueConfigData( );
  }
  else
#endif
    if ( argc < 4 )
  {
    std::cerr << "Error: an activity file must be provided after network file"
              << std::endl;
    return 1;
  }
  else
  {
    secondaryPath = argv[ 3 ];
    if ( simtype == "-csv" )
    {
      importer = new simil::LoadCSVData( );
    }
    else if ( simtype == "-h5" )
    {
      importer = new simil::LoadHDF5Data( );
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
      std::cerr << "-csv CSV loader " << std::endl;
      return 1;
    }
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

  return 0;
}