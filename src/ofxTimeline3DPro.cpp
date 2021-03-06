#include "ofxTimeline3DPro.h"

void ofxTimeline3DPro::play(){
	if(!bPlaying){
        ofx3DPro::play();
        
        timeline->setCurrentFrame(0);
		if(cameraTrack->getKeyframes().size() > 0){
			cameraTrack->lockCameraToTrack = true;
		}
		timeline->play();
	}
}

void ofxTimeline3DPro::stop(){
	if(bPlaying){
		timeline->stop();
		cameraTrack->lockCameraToTrack = false;
        ofx3DPro::stop();
	}
}

#define REZANATOR_GUI_ALPHA_MULTIPLIER 4

void ofxTimeline3DPro::update(ofEventArgs & args){
    if(bEnableTimeline){
        updateTimelineUIParams();
    }
    
    ofx3DPro::update(args);
	
	//Make this happen only when the timeline is modified by the user or when a new track is added.
	if(!ofGetMousePressed()){
		timeline->setOffset(ofVec2f(4, ofGetHeight() - timeline->getHeight() - 4 ));
		timeline->setWidth(ofGetWidth() - 8);
	}
}

void ofxTimeline3DPro::draw(ofEventArgs & args){
    
    ofx3DPro::draw(args);
    
	if(timeline != NULL){
        
        if ( timeline->getIsShowing() ){
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            ofPushStyle();
            ofFill();
            ofSetColor(0,100);
            ofRect(4, ofGetHeight() - timeline->getHeight() - 4, ofGetWidth() - 8, timeline->getHeight());
            ofPopStyle();
        }
        
		timeline->draw();
	}
}

void ofxTimeline3DPro::exit(ofEventArgs & args){
    if(logGui.isRecording()){
        logGui.record(false);
    }
    
    guiSave();
    
    //  3D Scene
    //
    lights.clear();
    materials.clear();
    guis.clear();
	
    //  TimeLine
    //
	if(cameraTrack != NULL){
        delete cameraTrack;
        cameraTrack = NULL;
    }
    if(timeline != NULL){
        delete timeline;
        timeline = NULL;
    }
    
    selfExit();
}

void ofxTimeline3DPro::keyPressed(ofKeyEventArgs & args){
    
	if(timeline->isModal()){
		return;
	}
    
    ofx3DPro::keyPressed(args);
	
    switch (args.key){
        case ' ':{
			timeline->togglePlay();
            ((ofxUIToggle *) tlGui->getWidget("ENABLE"))->setValue(timeline->getIsPlaying());
            ((ofxUIToggle *) tlGui->getWidget("ENABLE"))->triggerSelf();
        }
            break;
		case 'T':
			cameraTrack->addKeyframe();
			break;
		case 'L':
			cameraTrack->lockCameraToTrack = !cameraTrack->lockCameraToTrack;
			break;
    }
}

//-------------------------------------------------

bool ofxTimeline3DPro::cursorIsOverGUI(){
	if(timeline->getDrawRect().inside(ofGetMouseX(),ofGetMouseY())){
		return true;
	}

    return ofx2DPro::cursorIsOverGUI();
}

void ofxTimeline3DPro::setupCoreGuis(){
    ofx3DPro::setupCoreGuis();
    
    //  This is the specific of TIMELINE
    //
    setupTimeLineParams();
	setupTimeline();
    setupTimelineGui();
}

void ofxTimeline3DPro::setupTimeLineParams(){
	timeline = NULL;
    bShowTimeline = false;
	bTimelineIsIndefinite = true;
    bDeleteTimelineTrack = false;
    timelineDuration = 60;
    bEnableTimeline = false;
    bEnableTimelineTrackCreation = false;
}

void ofxTimeline3DPro::setupTimeline(){
    if(timeline != NULL){
        delete timeline;
        timeline = NULL;
    }
	
	timeline = new ofxTimeline();
	timeline->setName("Working");
	timeline->setWorkingFolder(getDataPath()+"Presets/Working/Timeline/");
	
	timeline->setup();
	timeline->setShowInoutControl(false);
    timeline->setMinimalHeaders(true);
	timeline->setFrameBased(false);
	timeline->setSpacebarTogglePlay(false);
	timeline->setDurationInSeconds(60);
	timeline->setLoopType(OF_LOOP_NONE);
    timeline->setPageName(ofToUpper(getSystemName()));
	
    if(cameraTrack!=NULL){
        delete cameraTrack;
    }
    
	cameraTrack = new ofxTLCameraTrack();
	cameraTrack->setCamera(getCameraRef());
	cameraTrack->setXMLFileName(getDataPath()+"Presets/Working/Timeline/cameraTrack.xml");
    timeline->addTrack("Camera", cameraTrack);
	
    ofDirectory dir;
    string workingDirectoryName = getDataPath()+"Presets/Working/Timeline/";
    if(!dir.doesDirectoryExist(workingDirectoryName)){
        dir.createDirectory(workingDirectoryName);
    }
    
    timeline->setWorkingFolder(getDataPath()+"Presets/Working/Timeline/");
    ofAddListener(timeline->events().bangFired, this, &ofxTimeline3DPro::timelineBangEvent);
    
    if(bShowTimeline){
        timeline->show();
    } else {
        timeline->hide();
    }
    
    selfSetupTimeline();
}

void ofxTimeline3DPro::resetTimeline(){
    
    if(timeline != NULL){
        ofRemoveListener(timeline->events().bangFired, this, &ofxTimeline3DPro::timelineBangEvent);
        timeline->reset();
    }
    
    if(cameraTrack != NULL){
        cameraTrack->disable();
        cameraTrack->lockCameraToTrack = false;
        delete cameraTrack;
        cameraTrack = NULL;
    }
    
    setupTimeline();
}

void ofxTimeline3DPro::timelineBangEvent(ofxTLBangEventArgs& args){
	
    if(bEnableTimeline){
        map<ofxTLBangs*, ofxUIButton*>::iterator it = tlButtonMap.find((ofxTLBangs *)args.track);
        if(it != tlButtonMap.end()) {
            ofxUIButton *b = it->second;
            b->setValue(!b->getValue());
            b->triggerSelf();
            b->setValue(!b->getValue());
        }
    }
}

void ofxTimeline3DPro::setupTimelineGui(){
    
    UIReference tmp( new ofxUISuperCanvas("TIMELINE", guiTemplate) );
    tlGui = tmp;
    tlGui->copyCanvasStyle(guiTemplate);
    tlGui->copyCanvasProperties(guiTemplate);
    tlGui->setPosition(guis[guis.size()-1]->getRect()->x+guis[guis.size()-1]->getRect()->getWidth()+1, 0);
    tlGui->setName("TimelineSettings");
    tlGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
	
    ofxUIToggle *toggle = tlGui->addToggle("ENABLE", &bEnableTimeline);
    toggle->setLabelPosition(OFX_UI_WIDGET_POSITION_LEFT);
    tlGui->resetPlacer();
    tlGui->addWidgetDown(toggle, OFX_UI_ALIGN_RIGHT, true);
    tlGui->addWidgetToHeader(toggle);
    tlGui->addSpacer();
    
    tlGui->addNumberDialer("DURATION", 0.0, 60*5, &timelineDuration, 1);//->setDisplayLabel(true);
    tlGui->addToggle("INDEFINITE", &bTimelineIsIndefinite);
    
    tlGui->addToggle("ANIMATE", &bEnableTimelineTrackCreation);
    tlGui->addToggle("DELETE", &bDeleteTimelineTrack);

    tlGui->addToggle("SHOW/HIDE", &bShowTimeline);
    
    selfSetupTimelineGui();
    tlGui->autoSizeToFitWidgets();
    ofAddListener(tlGui->newGUIEvent,this,&ofxTimeline3DPro::guiTimelineEvent);
    guis.push_back(tlGui);
}

void ofxTimeline3DPro::guiTimelineEvent(ofxUIEventArgs &e){
    string name = e.widget->getName();
    if(name == "DURATION"){
//		cout << "****** TL duration changed " << timelineDuration << endl;
        timeline->setDurationInSeconds(timelineDuration);
		timelineDuration = timeline->getDurationInSeconds();
    } else if(name == "ANIMATE"){
        ofxUIToggle *t = (ofxUIToggle *) e.widget;
        if(t->getValue()){
            setTimelineTrackDeletion(false);
        }
        setTimelineTrackCreation(t->getValue());
    } else if(name == "DELETE"){
        ofxUIToggle *t = (ofxUIToggle *) e.widget;
        if(t->getValue()){
            setTimelineTrackCreation(false);
        }
        setTimelineTrackDeletion(t->getValue());
        
    } else if(name == "ENABLE") {
        if(bEnableTimeline){
            if(bEnableTimelineTrackCreation){
                setTimelineTrackCreation(false);
            }
            
            if(bDeleteTimelineTrack){
                setTimelineTrackDeletion(false);
            }
        }
    } else if(name == "SHOW/HIDE"){
        if(bShowTimeline){
            timeline->show();
        } else {
            timeline->hide();
        }
    }
}

void ofxTimeline3DPro::setTimelineTrackDeletion(bool state){
    bDeleteTimelineTrack = state;
    if(bDeleteTimelineTrack){
        bEnableTimeline = false;
        for(vector<UIReference>::iterator it = guis.begin(); it != guis.end(); ++it){
            if((*it)->getName() != "TimelineSettings"){
                ofAddListener((*it)->newGUIEvent,this,&ofxTimeline3DPro::guiAllEvents);
            }
        }
    } else {
        for(vector<UIReference>::iterator it = guis.begin(); it != guis.end(); ++it){
            if((*it)->getName() != "TimelineSettings"){
                ofRemoveListener((*it)->newGUIEvent,this,&ofxTimeline3DPro::guiAllEvents);
            }
        }
    }
}

void ofxTimeline3DPro::setTimelineTrackCreation(bool state){
    bEnableTimelineTrackCreation = state;
    if(bEnableTimelineTrackCreation){
        bEnableTimeline = false;
        for(vector<UIReference>::iterator it = guis.begin(); it != guis.end(); ++it){
            if((*it)->getName() != "TimelineSettings"){
                ofAddListener((*it)->newGUIEvent,this,&ofxTimeline3DPro::guiAllEvents);
            }
        }
    } else {
        for(vector<UIReference>::iterator it = guis.begin(); it != guis.end(); ++it){
            if((*it)->getName() != "TimelineSettings"){
                ofRemoveListener((*it)->newGUIEvent,this,&ofxTimeline3DPro::guiAllEvents);
            }
        }
    }
}

void ofxTimeline3DPro::guiAllEvents(ofxUIEventArgs &e){
    //All GUIS except for the Timeline UI will send events to this function
    if(bEnableTimelineTrackCreation){
        bindWidgetToTimeline(e.widget);
        setTimelineTrackCreation(false);
    }
    
    if(bDeleteTimelineTrack){
        unBindWidgetFromTimeline(e.widget);
        setTimelineTrackDeletion(false);
    }
}
void ofxTimeline3DPro::bindWidgetToTimeline(ofxUIWidget* widget){
    string parentName = ((ofxUISuperCanvas *) widget->getCanvasParent())->getCanvasTitle()->getLabel();
    timeline->addPage(parentName, true);
    timeline->setCurrentPage(parentName);
    
    vector<ofxTLPage *> pages = timeline->getPages();
    
    ofxTLPage * currentPage = NULL;
    for(vector<ofxTLPage *>::iterator it = pages.begin(); it != pages.end(); ++it){
        if((*it)->getName() == parentName){
            currentPage = (*it);
        }
    }
	
    if(currentPage != NULL){
        if(currentPage->getTrack(widget->getName()) != NULL){
            return;
        }
    }
    
    switch (widget->getKind()){
        case OFX_UI_WIDGET_BUTTON:
        case OFX_UI_WIDGET_LABELBUTTON:
        case OFX_UI_WIDGET_IMAGEBUTTON:
        case OFX_UI_WIDGET_MULTIIMAGEBUTTON:
        {
            ofxUIButton *b = (ofxUIButton *) widget;
            tlButtonMap[timeline->addBangs(widget->getName())] = b;
        }
            break;
            
        case OFX_UI_WIDGET_TOGGLE:
        case OFX_UI_WIDGET_LABELTOGGLE:
        case OFX_UI_WIDGET_IMAGETOGGLE:
        case OFX_UI_WIDGET_MULTIIMAGETOGGLE:
        {
            ofxUIToggle *t = (ofxUIToggle *) widget;
            tlToggleMap[t] = timeline->addSwitches(widget->getName(),
												   widget->getCanvasParent()->getName() + "_" + widget->getName() + ".xml");
        }
            break;
            
        case OFX_UI_WIDGET_NUMBERDIALER:
        {
            ofxUINumberDialer *nd = (ofxUINumberDialer *) widget;
            tlDialerMap[nd] = timeline->addCurves(widget->getName(),
												  widget->getCanvasParent()->getName() + "_" + widget->getName() + ".xml",
												  ofRange(nd->getMin(), nd->getMax()), nd->getValue());
        }
            break;
            
        case OFX_UI_WIDGET_BILABELSLIDER:
        case OFX_UI_WIDGET_CIRCLESLIDER:
        case OFX_UI_WIDGET_IMAGESLIDER_H:
        case OFX_UI_WIDGET_IMAGESLIDER_V:
        case OFX_UI_WIDGET_ROTARYSLIDER:
        case OFX_UI_WIDGET_MINIMALSLIDER:
        case OFX_UI_WIDGET_SLIDER_H:
        case OFX_UI_WIDGET_SLIDER_V:
        {
            ofxUISlider *s = (ofxUISlider *) widget;
            tlSliderMap[s] = timeline->addCurves(widget->getName(),
												 widget->getCanvasParent()->getName() + "_" + widget->getName() + ".xml",
												 ofRange(s->getMin(), s->getMax()), s->getValue());
        }
            break;
        default:
            break;
    }
    
}

void ofxTimeline3DPro::unBindWidgetFromTimeline(ofxUIWidget* widget){
    string parentName = ((ofxUISuperCanvas *) widget->getCanvasParent())->getCanvasTitle()->getLabel();
    timeline->setCurrentPage(parentName);
    
    if(!timeline->hasTrack(widget->getName())){
        return;
    }
    
    switch (widget->getKind())
    {
        case OFX_UI_WIDGET_BUTTON:
        case OFX_UI_WIDGET_LABELBUTTON:
        case OFX_UI_WIDGET_IMAGEBUTTON:
        case OFX_UI_WIDGET_MULTIIMAGEBUTTON:
        {
            ofxTLBangs *track = (ofxTLBangs *) timeline->getTrack(widget->getName());
            map<ofxTLBangs *, ofxUIButton *>::iterator it = tlButtonMap.find(track);
            
            if(it != tlButtonMap.end())
            {
                timeline->removeTrack(it->first);
                tlButtonMap.erase(it);
            }
        }
            break;
            
        case OFX_UI_WIDGET_TOGGLE:
        case OFX_UI_WIDGET_LABELTOGGLE:
        case OFX_UI_WIDGET_IMAGETOGGLE:
        case OFX_UI_WIDGET_MULTIIMAGETOGGLE:
        {
            ofxUIToggle *t = (ofxUIToggle *) widget;
            map<ofxUIToggle *, ofxTLSwitches *>::iterator it = tlToggleMap.find(t);
            
            if(it != tlToggleMap.end())
            {
                timeline->removeTrack(it->second);
                tlToggleMap.erase(it);
            }
        }
            break;
            
        case OFX_UI_WIDGET_NUMBERDIALER:
        {
            ofxUINumberDialer *nd = (ofxUINumberDialer *) widget;
            map<ofxUINumberDialer *, ofxTLCurves *>::iterator it = tlDialerMap.find(nd);
            if(it != tlDialerMap.end())
            {
                timeline->removeTrack(it->second);
                tlDialerMap.erase(it);
            }
        }
            break;
            
        case OFX_UI_WIDGET_BILABELSLIDER:
        case OFX_UI_WIDGET_CIRCLESLIDER:
        case OFX_UI_WIDGET_IMAGESLIDER_H:
        case OFX_UI_WIDGET_IMAGESLIDER_V:
        case OFX_UI_WIDGET_ROTARYSLIDER:
        case OFX_UI_WIDGET_MINIMALSLIDER:
        case OFX_UI_WIDGET_SLIDER_H:
        case OFX_UI_WIDGET_SLIDER_V:
        {
            ofxUISlider *s = (ofxUISlider *) widget;
            map<ofxUISlider *, ofxTLCurves *>::iterator it = tlSliderMap.find(s);
            if(it != tlSliderMap.end()){
                timeline->removeTrack(it->second);
                tlSliderMap.erase(it);
            }
        }
            break;
        default:
            break;
    }
}

void ofxTimeline3DPro::updateTimelineUIParams()
{
    for(map<ofxUIToggle*, ofxTLSwitches*>::iterator it = tlToggleMap.begin(); it != tlToggleMap.end(); ++it){
        ofxUIToggle *t = it->first;
        ofxTLSwitches *tls = it->second;
        if(tls->isOn() != t->getValue()){
            t->setValue(tls->isOn());
            t->triggerSelf();
        }
    }
    
    for(map<ofxUISlider*, ofxTLCurves*>::iterator it = tlSliderMap.begin(); it != tlSliderMap.end(); ++it){
        ofxUISlider *s = it->first;
        ofxTLCurves *tlc = it->second;
        s->setValue(tlc->getValue());
        s->triggerSelf();
    }
    
    for(map<ofxUINumberDialer*, ofxTLCurves*>::iterator it = tlDialerMap.begin(); it != tlDialerMap.end(); ++it){
        ofxUINumberDialer *nd = it->first;
        ofxTLCurves *tlc = it->second;
        nd->setValue(tlc->getValue());
        nd->triggerSelf();
    }
}

void ofxTimeline3DPro::saveTimelineUIMappings(string path){
    if(ofFile::doesFileExist(path)){
		//        cout << "DELETING OLD MAPPING FILE" << endl;
        ofFile::removeFile(path);
    }
	//    cout << "TIMELINE UI MAPPER SAVING" << endl;
    ofxXmlSettings *XML = new ofxXmlSettings(path);
    XML->clear();
    
    int mapIndex = XML->addTag("Map");
    XML->pushTag("Map", mapIndex);
    
    int bangsIndex = XML->addTag("Bangs");
    if(XML->pushTag("Bangs", bangsIndex)){
        for(map<ofxTLBangs*, ofxUIButton*>::iterator it = tlButtonMap.begin(); it != tlButtonMap.end(); ++it){
            int index = XML->addTag("Mapping");
            if(XML->pushTag("Mapping", index)){
                int wIndex = XML->addTag("Widget");
                if(XML->pushTag("Widget", wIndex)){
                    XML->setValue("WidgetName", it->second->getName(), 0);
                    XML->setValue("WidgetID", it->second->getID(), 0);
                    XML->setValue("WidgetCanvasParent", it->second->getCanvasParent()->getName(), 0);
                    XML->popTag();
                }
                int tlIndex = XML->addTag("Track");
                if(XML->pushTag("Track", tlIndex)){
                    XML->popTag();
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }
    
    int switchesIndex = XML->addTag("Switches");
    if(XML->pushTag("Switches", switchesIndex)){
        for(map<ofxUIToggle*, ofxTLSwitches*>::iterator it = tlToggleMap.begin(); it != tlToggleMap.end(); ++it){
            int index = XML->addTag("Mapping");
            if(XML->pushTag("Mapping", index)){
                int wIndex = XML->addTag("Widget");
                if(XML->pushTag("Widget", wIndex)){
                    XML->setValue("WidgetName", it->first->getName(), 0);
                    XML->setValue("WidgetID", it->first->getID(), 0);
                    XML->setValue("WidgetCanvasParent", it->first->getCanvasParent()->getName(), 0);
                    XML->popTag();
                }
                int tlIndex = XML->addTag("Track");
                if(XML->pushTag("Track", tlIndex)){
                    XML->popTag();
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }
    
    int sliderCurvesIndex = XML->addTag("SliderCurves");
    if(XML->pushTag("SliderCurves", sliderCurvesIndex)){
        for(map<ofxUISlider*, ofxTLCurves*>::iterator it = tlSliderMap.begin(); it != tlSliderMap.end(); ++it){
            int index = XML->addTag("Mapping");
            if(XML->pushTag("Mapping", index)){
                int wIndex = XML->addTag("Widget");
                if(XML->pushTag("Widget", wIndex)){
                    XML->setValue("WidgetName", it->first->getName(), 0);
                    XML->setValue("WidgetID", it->first->getID(), 0);
                    XML->setValue("WidgetCanvasParent", it->first->getCanvasParent()->getName(), 0);
                    XML->popTag();
                }
                int tlIndex = XML->addTag("Track");
                if(XML->pushTag("Track", tlIndex)){
                    XML->popTag();
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }
    
    int numberDialerCurvesIndex = XML->addTag("NumberDialerCurves");
    if(XML->pushTag("NumberDialerCurves", numberDialerCurvesIndex)){
        for(map<ofxUINumberDialer*, ofxTLCurves*>::iterator it = tlDialerMap.begin(); it != tlDialerMap.end(); ++it){
            int index = XML->addTag("Mapping");
            if(XML->pushTag("Mapping", index)){
                int wIndex = XML->addTag("Widget");
                if(XML->pushTag("Widget", wIndex)){
                    XML->setValue("WidgetName", it->first->getName(), 0);
                    XML->setValue("WidgetID", it->first->getID(), 0);
                    XML->setValue("WidgetCanvasParent", it->first->getCanvasParent()->getName(), 0);
                    XML->popTag();
                }
                int tlIndex = XML->addTag("Track");
                if(XML->pushTag("Track", tlIndex)){
                    XML->popTag();
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }
    
    XML->popTag();
    XML->saveFile(path);
    delete XML;
}

void ofxTimeline3DPro::loadTimelineUIMappings(string path){
    tlButtonMap.clear();
    tlToggleMap.clear();
    tlSliderMap.clear();
    tlDialerMap.clear();
    
	//    cout << "LOADING TIMELINE UI MAPPINGS" << endl;
    ofxXmlSettings *XML = new ofxXmlSettings();
    XML->loadFile(path);
    if(XML->tagExists("Map",0) && XML->pushTag("Map", 0)){
        if(XML->pushTag("Bangs", 0)){
            int widgetTags = XML->getNumTags("Mapping");
            for(int i = 0; i < widgetTags; i++){
                if(XML->pushTag("Mapping", i)){
                    if(XML->pushTag("Widget", 0)){
                        
                        string widgetname = XML->getValue("WidgetName", "NULL", 0);
                        int widgetID = XML->getValue("WidgetID", -1, 0);
                        string widgetCanvasParent = XML->getValue("WidgetCanvasParent", "NULL", 0);
                        
                        for (int i = 0 ; i < guis.size(); i++) {
                            if (guis[i]->getName() == widgetCanvasParent ){
                                ofxUIWidget *w = guis[i]->getWidget(widgetname, widgetID);
                                if(w != NULL){
                                    bindWidgetToTimeline(w);
                                }
                            }
                        }
                        
                        XML->popTag();
                    }
                    XML->popTag();
                }
            }
            XML->popTag();
        }
        
        if(XML->pushTag("Switches", 0))
        {
            int widgetTags = XML->getNumTags("Mapping");
            for(int i = 0; i < widgetTags; i++)
            {
                if(XML->pushTag("Mapping", i))
                {
                    if(XML->pushTag("Widget", 0))
                    {
                        string widgetname = XML->getValue("WidgetName", "NULL", 0);
                        int widgetID = XML->getValue("WidgetID", -1, 0);
                        string widgetCanvasParent = XML->getValue("WidgetCanvasParent", "NULL", 0);
                        
                        for (int i = 0 ; i < guis.size(); i++) {
                            if (guis[i]->getName() == widgetCanvasParent ){
                                ofxUIWidget *w = guis[i]->getWidget(widgetname, widgetID);
                                if(w != NULL){
                                    bindWidgetToTimeline(w);
                                }
                            }
                        }
                        
                        XML->popTag();
                    }
                    XML->popTag();
                }
            }
            XML->popTag();
        }
        
        if(XML->pushTag("SliderCurves", 0))
        {
            int widgetTags = XML->getNumTags("Mapping");
            for(int i = 0; i < widgetTags; i++)
            {
                if(XML->pushTag("Mapping", i))
                {
                    if(XML->pushTag("Widget", 0))
                    {
                        string widgetname = XML->getValue("WidgetName", "NULL", 0);
                        int widgetID = XML->getValue("WidgetID", -1, 0);
                        string widgetCanvasParent = XML->getValue("WidgetCanvasParent", "NULL", 0);
                        
                        for (int i = 0 ; i < guis.size(); i++) {
                            if (guis[i]->getName() == widgetCanvasParent ){
                                ofxUIWidget *w = guis[i]->getWidget(widgetname, widgetID);
                                if(w != NULL){
                                    bindWidgetToTimeline(w);
                                }
                            }
                        }
                        
                        XML->popTag();
                    }
                    XML->popTag();
                }
            }
            XML->popTag();
        }
        
        if(XML->pushTag("NumberDialerCurves", 0)){
            int widgetTags = XML->getNumTags("Mapping");
            for(int i = 0; i < widgetTags; i++){
                if(XML->pushTag("Mapping", i)){
                    if(XML->pushTag("Widget", 0)){
                        string widgetname = XML->getValue("WidgetName", "NULL", 0);
                        int widgetID = XML->getValue("WidgetID", -1, 0);
                        string widgetCanvasParent = XML->getValue("WidgetCanvasParent", "NULL", 0);
                        
                        for (int i = 0 ; i < guis.size(); i++) {
                            if (guis[i]->getName() == widgetCanvasParent ){
                                ofxUIWidget *w = guis[i]->getWidget(widgetname, widgetID);
                                if(w != NULL){
                                    bindWidgetToTimeline(w);
                                }
                            }
                        }
                        
                        XML->popTag();
                    }
                    XML->popTag();
                }
            }
            XML->popTag();
        }
        XML->popTag();
    }
	
	timeline->setCurrentPage(0);
}


void ofxTimeline3DPro::guiLoad(){
    for(int i = 0; i < guis.size(); i++){
        guis[i]->loadSettings(getDataPath()+"Presets/Working/"+guis[i]->getName()+".xml");
    }
    camera->load(getDataPath()+"Presets/Working/"+"ofEasyCamSettings");
    
    resetTimeline();
    loadTimelineUIMappings(getDataPath()+"Presets/Working/UITimelineMappings.xml");
    timeline->loadTracksFromFolder(getDataPath()+"Presets/Working/Timeline/");
}

void ofxTimeline3DPro::guiSave(){
    ofx3DPro::guiSave();

    saveTimelineUIMappings(getDataPath()+"Presets/Working/UITimelineMappings.xml");
    timeline->saveTracksToFolder(getDataPath()+"Presets/Working/Timeline/");
}

void ofxTimeline3DPro::guiLoadPresetFromPath(string presetPath){
    resetTimeline();
    
    for(int i = 0; i < guis.size(); i++){
        guis[i]->loadSettings(presetPath+"/"+guis[i]->getName()+".xml");
    }
    camera->load(presetPath+"/ofEasyCamSettings");
	
    loadTimelineUIMappings(presetPath+"/UITimelineMappings.xml");
	timeline->setName( ofFilePath::getBaseName( presetPath ) );
    timeline->loadTracksFromFolder(presetPath+"/Timeline/");
	timeline->setName("Working");
    timeline->saveTracksToFolder(getDataPath()+"Presets/Working/Timeline/");

	timeline->setDurationInSeconds(timelineDuration);
	timelineDuration = timeline->getDurationInSeconds();
	
	selfPresetLoaded(presetPath);
	
	if(bTimelineIsIndefinite){
		timeline->setLoopType(OF_LOOP_NORMAL);
	} else {
		timeline->setLoopType(OF_LOOP_NONE);
	}
    
	if(bShowTimeline){
		UIReference last;
        for(vector<UIReference>::iterator it = guis.begin(); it != guis.end(); ++it){
            (*it)->setMinified(false);
            if(last != NULL){
                (*it)->getRect()->setX(last->getRect()->getX());
                (*it)->getRect()->setY(last->getRect()->getY()+last->getRect()->getHeight()+1);
                
                if((*it)->getRect()->getY()+(*it)->getRect()->getHeight() > ofGetHeight()-timeline->getHeight()){
                    (*it)->getRect()->setX(last->getRect()->getX()+last->getRect()->getWidth()+1);
                    (*it)->getRect()->setY(1);
                }
                
            } else {
                (*it)->getRect()->setX(1);
                (*it)->getRect()->setY(1);
            }
            last = (*it);
        }
	}
}

void ofxTimeline3DPro::guiSavePreset(string presetName){
    ofx3DPro::guiSavePreset(presetName);
	
    //  TimeLine specifics
    //
    saveTimelineUIMappings(getDataPath()+"Presets/"+presetName+"/UITimelineMappings.xml");
	timeline->setName(presetName);
    timeline->saveTracksToFolder(getDataPath()+"Presets/"+presetName+"/Timeline/");
	timeline->setName("Working");
    timeline->saveTracksToFolder(getDataPath()+"Presets/Working/Timeline/");

	ofxXmlSettings timeInfo;
	timeInfo.addTag("timeinfo");
	timeInfo.pushTag("timeinfo");
	timeInfo.addValue("indefinite", bTimelineIsIndefinite);
	timeInfo.addValue("duration", timelineDuration);
	timeInfo.popTag();//timeinfo
	timeInfo.saveFile(getDataPath()+"Presets/"+presetName+"/"+"TimeInfo.xml");
}

void ofxTimeline3DPro::guiShow(){
    ofx3DPro::guiShow();
	timeline->show();
}

void ofxTimeline3DPro::guiHide(){
    ofx3DPro::guiHide();
	timeline->hide();
}

void ofxTimeline3DPro::guiToggle(){
    ofx3DPro::guiToggle();
    
    bShowTimeline = !timeline->getIsShowing();
    
    if(bShowTimeline){
        timeline->show();
    } else {
        timeline->hide();
    }
}