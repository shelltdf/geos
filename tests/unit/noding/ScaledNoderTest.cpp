// 
// Test Suite for geos::noding::ScaledNoder class.

#include <tut.hpp>
// geos
#include <geos/noding/SegmentNode.h>
#include <geos/noding/ScaledNoder.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/snapround/MCIndexSnapRounder.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
// std
#include <memory>
#include <fstream>
#include <string>
#include <iomanip>
#include <iostream>


namespace tut
{
    //
    // Test Group
    //

    // Common data used by all tests
    struct test_scalednoder_data
    {

      typedef geos::noding::SegmentString SegmentString;
      typedef geos::noding::NodedSegmentString NodedSegmentString;
      typedef geos::noding::snapround::MCIndexSnapRounder MCIndexSnapRounder;
      typedef geos::noding::ScaledNoder ScaledNoder;
      typedef geos::io::WKTReader WKTReader;
      typedef geos::io::WKBReader WKBReader;
      typedef geos::io::WKTWriter WKTWriter;
      typedef geos::io::WKBWriter WKBWriter;
      typedef geos::geom::Geometry Geometry;
      typedef geos::geom::PrecisionModel PrecisionModel;
      typedef geos::geom::CoordinateSequence CoordinateSequence;

      typedef std::auto_ptr<CoordinateSequence> CoordSeqPtr;
      typedef std::auto_ptr<Geometry> GeomPtr;
	
      typedef std::vector<SegmentString*> SegStrVct;
      typedef std::vector<Geometry*> GeomVct;

      const geos::geom::CoordinateSequenceFactory* csf_;
      const geos::geom::GeometryFactory gf_;

      test_scalednoder_data()
            : csf_(geos::geom::CoordinateArraySequenceFactory::instance()),
              gf_()
      {}

      GeomPtr getGeometry(SegStrVct& vct)
      {
        GeomVct *lines = new GeomVct;
        for (SegStrVct::size_type i=0, n=vct.size(); i<n; ++i)
        {
          SegmentString* ss = vct[i];
          lines->push_back( gf_.createLineString(*(ss->getCoordinates())) );
        }
        return GeomPtr(gf_.createMultiLineString(lines));
      }

      std::string toWKT(SegStrVct& vct)
      {
        GeomPtr g = getGeometry(vct);
        WKTWriter w;
        w.setTrim(true);
        w.setRoundingPrecision(10);
        return w.write(g.get());
      }

      std::string toHEXWKB(SegStrVct& vct)
      {
        GeomPtr g = getGeometry(vct);
        WKBWriter w;
	std::stringstream ss;
        w.writeHEX(*g, ss);
	return ss.str();
      }

      void getSegmentStrings(const Geometry& g, SegStrVct& vct)
      {
        CoordSeqPtr s ( g.getCoordinates() );
        vct.push_back( new NodedSegmentString(s.release(), 0) );
      }

    GeomPtr readGeometry(const std::string& wkt)
    {
      GeomPtr g;
      if ( wkt[0] == '0' || wkt[0] == '1' ) {
        WKBReader r;
        std::istringstream is(wkt);
        g.reset( r.readHEX(is) );
      } else {
        WKTReader r;
        g.reset( r.read(wkt) );
      }
      return g;
    }


    void getSegmentStrings(const std::string& wkt, SegStrVct& vct)
    {
      GeomPtr g = readGeometry(wkt);
        getSegmentStrings(*g, vct);
      }

      void freeSegmentStrings(SegStrVct& vct)
      {
        for (SegStrVct::size_type i=0, n=vct.size(); i<n; ++i)
          delete vct[i];
      }

      std::string readFileAsString(const std::string& path)
      {
        std::ifstream ifs(path.c_str());
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ) );
        return content;
      }
    };

    typedef test_group<test_scalednoder_data> group;
    typedef group::object object;

    group test_scalednoder_group("geos::noding::ScaledNoder");

    //
    // Test Cases
    //

#if 0
    // Test wrapping MCIndexSnapRounder as happens with BufferBuilder
    // Scale: 1
    template<>
    template<>
    void object::test<1>()
    {
        const char* wkti = "LINESTRING(0 0, 10 0,5 5, 5 -5)";
        SegStrVct nodable;
        getSegmentStrings(wkti, nodable);

        PrecisionModel pm(1.0); // fixed as well
        MCIndexSnapRounder inoder(pm);
        ScaledNoder noder(inoder, 1.0); // scale=1.0

        noder.computeNodes(&nodable);
	      SegStrVct *noded = noder.getNodedSubstrings();

        ensure_equals( noded->size(), 5u );

        std::string wktobt = toWKT(*noded);

        const char* wktexp = "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (10 0, 5 5), (5 5, 5 0), (5 0, 5 -5))";
        ensure_equals( wktobt, wktexp );

        freeSegmentStrings(nodable);
        freeSegmentStrings(*noded); delete noded;

    }

    // Test wrapping MCIndexSnapRounder as happens with BufferBuilder
    // Scale: 1e5
    template<>
    template<>
    void object::test<2>()
    {
        const char* wkti = "LINESTRING(0 0, 10 0,5 5, 5 -5)";
        SegStrVct nodable;
        getSegmentStrings(wkti, nodable);

        PrecisionModel pm(1.0); // fixed as well
        MCIndexSnapRounder inoder(pm);
        ScaledNoder noder(inoder, 1e5); 

        noder.computeNodes(&nodable);
	      SegStrVct *noded = noder.getNodedSubstrings();

        ensure_equals( noded->size(), 5u );

        std::string wktobt = toWKT(*noded);

        const char* wktexp = "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (10 0, 5 5), (5 5, 5 0), (5 0, 5 -5))";
        ensure_equals( wktobt, wktexp );

        freeSegmentStrings(nodable);
        freeSegmentStrings(*noded); delete noded;

    }
#endif

    // Test wrapping MCIndexSnapRounder as happens with BufferBuilder
    // Scale: 1e5
    template<>
    template<>
    void object::test<3>()
    {

      //std::string wkti = readFileAsString("/tmp/diffnoding.wkt");
      //std::string wkti = readFileAsString("/tmp/bufrob3.wkt");
      //std::string wkti = readFileAsString("/tmp/bufrob3_5.wkt");
      std::string wkti = readFileAsString("/tmp/bufrob4.wkt");

      SegStrVct nodable;
      getSegmentStrings(wkti, nodable);

      PrecisionModel pm(1.0); // fixed as well
      MCIndexSnapRounder inoder(pm);
      ScaledNoder noder(inoder, 1e5); 

      noder.computeNodes(&nodable);
      SegStrVct *noded = noder.getNodedSubstrings();

std::cout << toHEXWKB(*noded) << std::endl;
      //ensure_equals( noded->size(), 1058u ); // bufrob2
      //ensure_equals( noded->size(), 333u ); // bufrob3
      //ensure_equals( noded->size(), 147u ); // bufrob3_5
      ensure_equals( noded->size(), 19u ); // bufrob4

#if 0
	//std::cout << toWKT(*noded) << std::endl;
        std::string wktobt = toWKT(*noded);

        const char* wktexp = "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (10 0, 5 5), (5 5, 5 0), (5 0, 5 -5))";
        ensure_equals( wktobt, wktexp );
#endif

        freeSegmentStrings(nodable);
        freeSegmentStrings(*noded); delete noded;

    }


} // namespace tut
