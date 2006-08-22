/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:41 CET 2006  eventservice_test.cpp 

                        eventservice_test.cpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#include <rtt/Event.hpp>
#include <rtt/Logger.hpp>
#include <rtt/RunnableInterface.hpp>
#include <rtt/SimulationActivity.hpp>
#include <rtt/SimulationThread.hpp>
#include <rtt/NonPeriodicActivity.hpp>

#include <rtt/EventService.hpp>
#include <rtt/DataSourceGenerator.hpp>

#include "eventservice_test.hpp"
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( EventServiceTest );

using namespace RTT;
using namespace RTT;
using namespace boost;
using namespace std;

void 
EventServiceTest::setUp()
{
    t_event0 = Event<void(void)>("t_event0");
    t_event1 = RTT::Event<void( std::string )>("t_event1");
    t_event2 = RTT::Event<void( std::string, double )>("t_event2");
    t_event3 = RTT::Event<void( std::string, double, bool )>("t_event3");

    
    event_proc = new EventProcessor();
    act.run(event_proc);
    act.start();
    es = new EventService(event_proc);
    reset();
}


void 
EventServiceTest::tearDown()
{
    act.stop();
    delete es;
    delete event_proc;
}

void EventServiceTest::listener0(void)
{
    t_listener_done = true;
}

void EventServiceTest::completer0(void)
{
    t_completer_done = true;
}

void EventServiceTest::listener1(const std::string& s)
{
    t_listener_done = true;
    t_listener_string=s;
}
void EventServiceTest::completer1(const std::string&s) 
{
    t_completer_done = true;
    t_completer_string=s;
}
void EventServiceTest::listener2(const std::string& s, double f )
{
    t_listener_done = true;
    t_listener_string=s;
    t_listener_double=f;
}
void EventServiceTest::completer2(const std::string& s, double f)
{
    t_completer_done = true;
    t_completer_string=s;
    t_completer_double=f;
}

void EventServiceTest::listener3(std::string s, double f, bool b)
{
    t_listener_done = true;
    t_listener_string=s;
    t_listener_double=f;
    t_listener_bool=b;
}
void EventServiceTest::completer3(std::string s, double f, bool b )
{
    t_completer_done = true;
    t_completer_string=s;
    t_completer_double=f;
    t_completer_bool=b;
}

void EventServiceTest::reset()
{
    t_listener_done = false;
    t_completer_done = false;
    t_listener_string = "";
    t_listener_double = 0.0;
    t_listener_bool = false;
    t_completer_string = "";
    t_completer_double = 0.0;
    t_completer_bool = false;
}

void EventServiceTest::setup()
{
    es->addEvent( &t_event0, "");
    es->addEvent( &t_event1, "", "", "" );
    es->addEvent( &t_event2, "", "", "", "", "" );
    es->addEvent( &t_event3, "", "", "", "", "", "", "" );
}

void EventServiceTest::cleanup()
{
    es->removeEvent( "t_event0" );
    es->removeEvent( "t_event1" );
    es->removeEvent( "t_event2" );
    es->removeEvent( "t_event3" );
}

void EventServiceTest::testAddRemove()
{
    bool result;
    result = es->addEvent( &t_event0 );
    CPPUNIT_ASSERT( result );
    result = es->addEvent( &t_event1 );
    CPPUNIT_ASSERT( result );
    result = es->addEvent( &t_event2 );
    CPPUNIT_ASSERT( result );
    result = es->addEvent( &t_event3 );
    CPPUNIT_ASSERT( result );

    result = es->addEvent( &t_event0 );
    CPPUNIT_ASSERT( result == false );

    result = es->removeEvent( "t_event0" );
    CPPUNIT_ASSERT( result );
    result = es->removeEvent( "t_event1" );
    CPPUNIT_ASSERT( result );
    result = es->removeEvent( "t_event2" );
    CPPUNIT_ASSERT( result );
    result = es->removeEvent( "t_event3" );
    CPPUNIT_ASSERT( result );

    result = es->removeEvent( "t_event0" );
    CPPUNIT_ASSERT( result == false );
}

void EventServiceTest::testSetupSyn()
{
    this->setup();
    Handle h;
    h = es->setupSyn("t_event0", bind(&EventServiceTest::listener0,this), vector<DataSourceBase::shared_ptr>() );
    CPPUNIT_ASSERT( h );
    h = es->setupSyn("t_event1", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string)));
    CPPUNIT_ASSERT( h );
    h = es->setupSyn("t_event2", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string), ref(t_listener_double)));
    CPPUNIT_ASSERT( h );
    h = es->setupSyn("t_event3", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string), ref(t_listener_double),ref(t_listener_bool)));
    CPPUNIT_ASSERT( h );

    this->cleanup();
}

void EventServiceTest::testSetupAsyn()
{
    this->setup();
    Handle h;
    h = es->setupAsyn("t_event0", bind(&EventServiceTest::completer0,this), vector<DataSourceBase::shared_ptr>(),event_proc );
    CPPUNIT_ASSERT( h );
    h = es->setupAsyn("t_event1", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string)),event_proc);
    CPPUNIT_ASSERT( h );
    h = es->setupAsyn("t_event2", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string), ref(t_completer_double)),event_proc);
    CPPUNIT_ASSERT( h );
    h = es->setupAsyn("t_event3", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string), ref(t_completer_double), ref(t_completer_bool) ),event_proc);
    CPPUNIT_ASSERT( h );

    this->cleanup();
}

void EventServiceTest::testSetupEmit()
{
    this->setup();

    ActionInterface::shared_ptr r;
    r.reset( es->getEvent("t_event0", std::vector<DataSourceBase::shared_ptr>() ) );
    CPPUNIT_ASSERT( r );
    r.reset( es->getEvent("t_event1", GenerateDataSource()(std::string("hello")) ) );
    CPPUNIT_ASSERT( r );
    r.reset( es->getEvent("t_event2", GenerateDataSource()(std::string("hello"),0.1234) ) );
    CPPUNIT_ASSERT( r );
    r.reset( es->getEvent("t_event3", GenerateDataSource()(std::string("hello"),0.1234, true) ) );
    CPPUNIT_ASSERT( r );

    this->cleanup();
}

void EventServiceTest::testEmit0()
{
    this->setup();

    Handle h1, h2;
    h1 = es->setupSyn("t_event0", bind(&EventServiceTest::listener0,this), vector<DataSourceBase::shared_ptr>() );
    h2 = es->setupAsyn("t_event0", bind(&EventServiceTest::completer0,this), vector<DataSourceBase::shared_ptr>(),event_proc );
    ActionInterface::shared_ptr r;
    r.reset( es->getEvent("t_event0", std::vector<DataSourceBase::shared_ptr>() ) );

    CPPUNIT_ASSERT( h1.connect() );
    r->execute();
    CPPUNIT_ASSERT( t_listener_done );
    this->reset();
    CPPUNIT_ASSERT( h1.disconnect() );

    CPPUNIT_ASSERT( h2.connect() );
    r->execute();
    CPPUNIT_ASSERT( !t_completer_done );
    event_proc->step();
    CPPUNIT_ASSERT( t_completer_done );
    this->reset();
    CPPUNIT_ASSERT( h2.disconnect() );

    this->cleanup();
}

void EventServiceTest::testEmit1()
{
    this->setup();

    Handle h1, h2;
    ActionInterface::shared_ptr r;

    h1 = es->setupSyn("t_event1", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string)));
    h2 = es->setupAsyn("t_event1", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string)),event_proc);
    r.reset( es->getEvent("t_event1", GenerateDataSource()(std::string("hello")) ) );

    CPPUNIT_ASSERT( h1.connect() );
    r->execute();
    CPPUNIT_ASSERT( t_listener_done );
    CPPUNIT_ASSERT( t_listener_string == std::string("hello") );
    this->reset();
    CPPUNIT_ASSERT( h1.disconnect() );

    CPPUNIT_ASSERT( h2.connect() );
    r->execute();
    CPPUNIT_ASSERT( !t_completer_done );
    event_proc->step();
    CPPUNIT_ASSERT( t_completer_done );
    CPPUNIT_ASSERT( t_completer_string == std::string("hello") );
    this->reset();
    CPPUNIT_ASSERT( h2.disconnect() );

    this->cleanup();
}
void EventServiceTest::testEmit2()
{
    this->setup();
    Handle h1, h2;
    ActionInterface::shared_ptr r;

    h1 = es->setupSyn("t_event2", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string), ref(t_listener_double)));
    h2 = es->setupAsyn("t_event2", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string), ref(t_completer_double)),event_proc);

    r.reset( es->getEvent("t_event2", GenerateDataSource()(std::string("hello"),0.1234) ) );


    CPPUNIT_ASSERT( h1.connect() );
    r->execute();
    CPPUNIT_ASSERT( t_listener_done );
    CPPUNIT_ASSERT( t_listener_string == std::string("hello") );
    CPPUNIT_ASSERT( t_listener_double == 0.1234 );
    this->reset();
    CPPUNIT_ASSERT( h1.disconnect() );

    CPPUNIT_ASSERT( h2.connect() );
    r->execute();
    CPPUNIT_ASSERT( !t_completer_done );
    event_proc->step();
    CPPUNIT_ASSERT( t_completer_done );
    CPPUNIT_ASSERT( t_completer_string == std::string("hello") );
    CPPUNIT_ASSERT( t_completer_double == 0.1234 );
    this->reset();
    CPPUNIT_ASSERT( h2.disconnect() );

    this->cleanup();
}

void EventServiceTest::testEmit3()
{
    this->setup();
    Handle h1, h2;
    ActionInterface::shared_ptr r;

    h1 = es->setupSyn("t_event3", bind(&EventServiceTest::listener0,this), GenerateDataSource()(ref(t_listener_string), ref(t_listener_double),ref(t_listener_bool)));
    h2 = es->setupAsyn("t_event3", bind(&EventServiceTest::completer0,this), GenerateDataSource()(ref(t_completer_string), ref(t_completer_double), ref(t_completer_bool) ),event_proc);
    r.reset( es->getEvent("t_event3", GenerateDataSource()(std::string("hello"),0.1234, true) ) );


    CPPUNIT_ASSERT( h1.connect() );
    r->execute();
    CPPUNIT_ASSERT( t_listener_done );
    CPPUNIT_ASSERT( t_listener_string == std::string("hello") );
    CPPUNIT_ASSERT( t_listener_double == 0.1234 );
    CPPUNIT_ASSERT( t_listener_bool == true );
    this->reset();
    CPPUNIT_ASSERT( h1.disconnect() );

    CPPUNIT_ASSERT( h2.connect() );
    r->execute();
    CPPUNIT_ASSERT( !t_completer_done );
    event_proc->step();
    CPPUNIT_ASSERT( t_completer_done );
    CPPUNIT_ASSERT( t_completer_string == std::string("hello") );
    CPPUNIT_ASSERT( t_completer_double == 0.1234 );
    CPPUNIT_ASSERT( t_completer_bool == true );
    this->reset();
    CPPUNIT_ASSERT( h2.disconnect() );

    this->cleanup();
}

void Foo3(string, double, bool)
{
}

void EventServiceTest::testEventC()
{
    // Test EventC and ConnectionC...
    Handle h1, h2, h3;

    this->setup();

    try {
        //h1 = es->setupConnection("t_event3").callback( Foo3 ).handle();
        h1 = es->setupConnection("t_event3").callback( this, &EventServiceTest::listener3 ).handle();
    } catch ( std::exception& e ) {
        CPPUNIT_ASSERT_MESSAGE( e.what(), false );
    }

    try {
        h2 = es->setupConnection("t_event3").callback( this, &EventServiceTest::completer3 ,event_proc).handle();
    } catch ( std::exception& e ) {
        CPPUNIT_ASSERT_MESSAGE( e.what(), false );
    }

    EventC evc;
    bool evcarg = true;
    try {
        evc = es->setupEmit("t_event3").argC( std::string("hello") ).argC( 0.1234 ).arg( evcarg );
    } catch ( std::exception& e ) {
        CPPUNIT_ASSERT_MESSAGE( e.what(), false );
    }

    CPPUNIT_ASSERT( h1.connect() );
    evc.emit();
    CPPUNIT_ASSERT( t_listener_done );
    CPPUNIT_ASSERT( t_listener_string == std::string("hello") );
    CPPUNIT_ASSERT( t_listener_double == 0.1234 );
    CPPUNIT_ASSERT( t_listener_bool == evcarg );
    this->reset();
    CPPUNIT_ASSERT( h1.disconnect() );

    CPPUNIT_ASSERT( h2.connect() );
    evc.emit();
    CPPUNIT_ASSERT( !t_completer_done );
    event_proc->step();
    CPPUNIT_ASSERT( t_completer_done );
    CPPUNIT_ASSERT( t_completer_string == std::string("hello") );
    CPPUNIT_ASSERT( t_completer_double == 0.1234 );
    CPPUNIT_ASSERT( t_completer_bool == evcarg );
    this->reset();
    CPPUNIT_ASSERT( h2.disconnect() );

    this->cleanup();

}

