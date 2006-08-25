#include "rtt/marsh/TinyDemarshaller.hpp"


// Modified tinyxml* to include it in the RTT namespace to avoid clashes
// with possible other libraries.
#include "tinyxml.h"

#ifdef TIXML_USE_STL
#include <iostream>
#include <sstream>
using namespace std;
#else
#include <cstdio>
#endif

#include <stack>
#include <rtt/Property.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/Logger.hpp>

namespace RTT
{
    namespace detail
    {
        class Tiny2CPFHandler 
        {
            /**
             * Stores the results of the parsing.
             */
            PropertyBag &bag;
            std::stack< std::pair<PropertyBag*, Property<PropertyBag>*> > bag_stack;

            enum Tag { TAG_STRUCT, TAG_SIMPLE, TAG_SEQUENCE, TAG_PROPERTIES, TAG_DESCRIPTION, TAG_VALUE, TAG_UNKNOWN};
            std::stack<Tag> tag_stack;

            /**
             * The name of the property.
             */
            std::string name;
            std::string description;
            std::string type;
            std::string value_string;

        public:

            Tiny2CPFHandler( PropertyBag &b ) : bag( b )
            {
                Property<PropertyBag>* dummy = 0;
                bag_stack.push(std::make_pair(&bag, dummy));
            }

            bool endElement()
            {
                switch ( tag_stack.top() )
                    {
                    case TAG_SIMPLE:
                        if ( type == "boolean" )
                        {
                            if ( value_string == "1" )
                                bag_stack.top().first->add
                                ( new Property<bool>( name, description, true ) );
                            else if ( value_string == "0" )
                                bag_stack.top().first->add
                                ( new Property<bool>( name, description, false ) );
                            else {
                                log(Error)<< "Wrong value for property '"+type+"'." \
                                    " Value should contain '0' or '1', got '"+ value_string +"'." << endlog();
                                return false;
                            }
                        }
                        else if ( type == "char" ) {
                            if ( value_string.length() != 1 ) {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain a single character, got '"+ value_string +"'." << endlog();
                                return false;
                            }
                            else 
                                bag_stack.top().first->add
                                    ( new Property<char>( name, description, value_string[0] ) );
                        }
                        else if ( type == "uchar" ) {
                            if ( value_string.length() != 1 ) {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain a single unsigned character, got '"+ value_string +"'." << endlog();
                                return false;
                            }
                            else 
                                bag_stack.top().first->add
                                    ( new Property<unsigned char>( name, description, value_string[0] ) );
                        }
                        else if ( type == "long" || type == "short") 
                        {
                            int v;
                            if ( sscanf(value_string.c_str(), "%d", &v) == 1)
                                bag_stack.top().first->add( new Property<int>( name, description, v ) );
                            else {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain an integer value, got '"+ value_string +"'." << endlog();
                                return false;
                            }
                        }
                        else if ( type == "ulong" || type == "ushort") 
                        {
                            unsigned int v;
                            if ( sscanf(value_string.c_str(), "%u", &v) == 1)
                                bag_stack.top().first->add( new Property<unsigned int>( name, description, v ) );
                            else {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain an integer value, got '"+ value_string +"'." << endlog();
                                return false;
                            }
                        }
                        else if ( type == "double") 
                        {
                            double v;
                            if ( sscanf(value_string.c_str(), "%lf", &v) == 1 )
                                bag_stack.top().first->add
                                    ( new Property<double>( name, description, v ) );
                            else {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain a double value, got '"+ value_string +"'." << endlog();
                                return false;
                            } 
                        }
                        else if ( type == "float") 
                        {
                            float v;
                            if ( sscanf(value_string.c_str(), "%f", &v) == 1 )
                                bag_stack.top().first->add
                                    ( new Property<float>( name, description, v ) );
                            else {
                                log(Error) << "Wrong value for property '"+type+"'." \
                                    " Value should contain a float value, got '"+ value_string +"'." << endlog();
                                return false;
                            }
                        }
                        else if ( type == "string") 
                            bag_stack.top().first->add
                            ( new Property<std::string>( name, description, value_string ) );
                        tag_stack.pop();
                        value_string.clear(); // cleanup
                        description.clear();
                        name.clear();
                        break;

                    case TAG_SEQUENCE:
                    case TAG_STRUCT:
                        {
                            Property<PropertyBag>* prop = bag_stack.top().second;
                            bag_stack.pop();
                            bag_stack.top().first->add( prop );
                            //( new Property<PropertyBag>( pn, description, *pb ) );
                            //delete pb;
                            tag_stack.pop();
                            description.clear();
                            name.clear();
                            type.clear();
                        }
                        break;

                    case TAG_DESCRIPTION:
                        tag_stack.pop();
                        if ( tag_stack.top() == TAG_STRUCT ) {
                            // it is a description of a struct that ended
                            bag_stack.top().second->setDescription(description);
                            description.clear();
                        }
                        break;
                    case TAG_VALUE:
                    case TAG_PROPERTIES:
                    case TAG_UNKNOWN:
                        tag_stack.pop();
                        break;

                    }
                return true;
            }


            void startElement(const char* localname,
                              const TiXmlAttribute* attributes )
            {
                std::string ln = localname;

                if ( ln == "properties" )
                    tag_stack.push( TAG_PROPERTIES );
                else
                    if ( ln == "simple" ) 
                    {
                        tag_stack.push( TAG_SIMPLE );
                        while (attributes)
                        {
                            std::string an = attributes->Name();
                            if ( an == "name") 
                            {
                                name = attributes->Value();
                            }
                            else if ( an == "type")
                            {
                                type = attributes->Value();
                            }
                            attributes = attributes->Next();
                        }
                    }
                    else
                        if ( ln == "struct" || ln == "sequence")
                        {
                            if ( ln == "struct" )
                                tag_stack.push( TAG_STRUCT );
                            else 
                                tag_stack.push( TAG_SEQUENCE );

                            //type tag is optional (?)
                            bool hasType = false;
                            while (attributes)
                                {
                                    std::string an = attributes->Name();
                                    if ( an == "name") 
                                        {
                                            name = attributes->Value();
                                        }
                                    else if ( an == "type")
                                        {
                                            hasType = true;
                                            type = attributes->Value();
                                        }
                                    attributes = attributes->Next();
                                }
                            Property<PropertyBag> *prop;
                            if (hasType)
                                prop = new Property<PropertyBag>(name,"",PropertyBag(type));
                            else
                                prop = new Property<PropertyBag>(name,"");
                            
                            // take reference to bag itself !
                            bag_stack.push(std::make_pair( &(prop->value()), prop));
                        }
                        else
                                if ( ln == "description") 
                                    tag_stack.push( TAG_DESCRIPTION );
                                else
                                    if ( ln == "value"  )
                                        tag_stack.push( TAG_VALUE );
                                    else {
                                        log(Warning) << "Unrecognised XML tag :"<< ln <<": ignoring." << endlog();
                                        tag_stack.push( TAG_UNKNOWN );
                                    }
            }

            void characters( const char* chars )
            {
                switch ( tag_stack.top() )
                {
                    case TAG_DESCRIPTION:
                        description = chars;
                        break;

                    case TAG_VALUE:
                        value_string = chars;;
                        break;
                    case TAG_STRUCT:
                    case TAG_SIMPLE:
                    case TAG_SEQUENCE:
                    case TAG_PROPERTIES:
                    case TAG_UNKNOWN:
                        break;
                }
            }

            bool populateBag(TiXmlNode* pParent)
            {
                if ( !pParent )
                    return false;

                TiXmlNode* pChild;
                TiXmlText* pText;
                int t = pParent->Type();
        
                switch ( t )
                    {
                    case TiXmlNode::ELEMENT:
                        // notify start of new element
                        this->startElement( pParent->Value(), pParent->ToElement()->FirstAttribute() );

                        // recurse in children, if any
                        for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
                            {
                                if ( this->populateBag( pChild ) == false)
                                    return false;
                            }

                        // notify end of element
                        if ( this->endElement() == false )
                            return false;
                        break;

                    case TiXmlNode::TEXT:
                        pText = pParent->ToText();
                        this->characters( pText->Value() );
                        break;

                        // not interested in these...
                    case TiXmlNode::DECLARATION:
                    case TiXmlNode::COMMENT:
                    case TiXmlNode::UNKNOWN:
                    case TiXmlNode::DOCUMENT:
                    default:
                        break;
                    }
            return true;
            }
        };
    }
     
    using namespace detail;

    struct TinyDemarshaller::D {
        D(const std::string& f) : doc( f.c_str() ), loadOkay(false) {}
        TiXmlDocument doc;
        bool loadOkay;
    };

    TinyDemarshaller::TinyDemarshaller( const std::string& filename )
        : d( new TinyDemarshaller::D(filename) )
    {
        Logger::In in("TinyDemarshaller");
        d->loadOkay = d->doc.LoadFile();

        if ( !d->loadOkay ) {
            log(Error) << "Could not load " << filename << " Error: "<< d->doc.ErrorDesc() << endlog();
            return;
        }

    }

    TinyDemarshaller::~TinyDemarshaller()
    {
        delete d;
    }
    
    bool TinyDemarshaller::deserialize( PropertyBag &v )
    {
        Logger::In in("TinyDemarshaller");

        if ( !d->loadOkay )
            return false;

		TiXmlHandle docHandle( &d->doc );
		TiXmlHandle propHandle = docHandle.FirstChildElement( "properties" );

        if ( ! propHandle.Node() ) {
            log(Error) << "No <properties> element found in document!"<< endlog();
            return false;
        }

        detail::Tiny2CPFHandler proc( v );

        if ( proc.populateBag( propHandle.Node() ) == false) {
            deleteProperties( v );
            return false;
        }
        return true;
    }

}
