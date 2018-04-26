program Lab7x3xd;

{$R-}

uses GraphABC, System.Windows.Forms, System.Drawing;

// Constants
const
    cMaxObjectSpaceSize    : Word = 19;  // +1
    cMaxWorldSpaceSize     : Word = 255; // +1
    cMaxObjectPolygons     : Word = 15;  // +1
    cMaxWorldPolygons      : Word = 127; // +1
    
    cPi                    : Real = 3.1415927;
    cPiDiv2                : Real = 1.5707963;
    c2Pi                   : Real = 6.2831853;
    c3PiDiv2               : Real = 4.7123890;
    cRadToDeg              : Real = 57.295780;
    
    cDefaultFramesPerSecond: Byte = 60;

// Types
type
    TVector3       = array [0..2]       of Real;
    TMatrix3x3     = array [0..2, 0..2] of Real;
    TVertices      = array [0..cMaxObjectSpaceSize] of TVector3;
    TSceneVertices = array [0..cMaxWorldSpaceSize]  of TVector3;
    
    TPolyIndecies  = array [0..2] of Word;
    TPolygon = record
        indecies: TPolyIndecies;
        penColor: Color;
        
        procedure setValues(i0, i1, i2: Word; pen_color: Color);
    end;
    
    TPolygons      = array [0..cMaxObjectPolygons ] of TPolygon;
    TScenePolygons = array [0..cMaxWorldPolygons]   of TPolygon;
    
    TPoint     = record
        x: Real;
        y: Real;
    end;
    
    TTransformations = record
        scale       : Real; 
        angle       : Real;
        displacement: TPoint;
    end;
    
    TScene     = record
        vertices   : TSceneVertices;
        indices    : TScenePolygons;
        worldMatrix: TMatrix3x3;
    
        numOfVertices: Word; // +1
        numOfIndices : Word; // +1
        
        constructor create();
        begin
            numOfVertices := 0;
            numOfIndices  := 0;
        end;
    end;
    
    TObject    = record
        vertices : TVertices;
        indices  : TPolygons;
        
        objectMatrix: TMatrix3x3;
        
        numOfVertices: Word; // +1
        numOfIndices : Word; // +1
    end;
    
    TRender = record
        scene     : TScene;
        viewMatrix: TMatrix3x3;
        
        numberOfObjects : Word;
        lastVertexIndex : Word;
        lastPolygonIndex: Word;

        procedure refresh();
        procedure addObject(obj: TObject);
        procedure renderFrame();
    end;
    
    TFlightMode = (fmWaiting, 
                   fmCranking, // Engine start
                   fmLaunch, 
                   fmOrbit);
    
    TRocket = record
        rocket   : TObject;
        transform: TTransformations;
        
        currentTime              : Real; // Second
        angle                    : Real; // Radians
        angularVelocity          : Real; // Radians per second
        angularAcceleration      : Real; // Radians per second^2
        orbitAltitude            : Real; // Meters
        orbitAltitudeSpeed       : Real; // Meters per second
        orbitAltitudeAcceleration: Real; // Meters per second^2
        orbitAngle               : Real; // Radians
        orbitAngleVelocity       : Real; // Radians per second
        orbitAngularAcceleration : Real; // Radians per second^2
        
        waitingTime              : Real; // Second
        crankingTime             : Real; // Second
        launchTime               : Real; // Second
        
        startOrbitAltitude       : Real; // Meters
        targetOrbitAltitude      : Real; // Meters
        maxLaunchAngle           : Real; // Radians
        
        mode      : TFlightMode;
        
        procedure createBody();
        procedure createFire();
        procedure createAll();
        procedure changeFire();
        procedure changeState(delta_time: Real);
    end;
    
    TLabAnimation  = record
        render: TRender;
      
        rocket: TRocket;
        
        scaleView  : Real;
        worldWidth : Real;
        worldHeight: Real;
        
        frameTime : Real;
        timerStart: Boolean;
        
        constructor create(fps: Byte; start_orbit, target_orbit, wait_time, 
            cranking_time, launch_time, max_launch_angle: Real);
        procedure setOrbitParams(start_orbit, target_orbit: Real);
        procedure createObjects();
        procedure setFPS(fps: Byte);
        procedure addObject(world_transform: TTransformations; 
            var obj: TObject);
        procedure updateScene(delta_time: Real);
        procedure drawBackGround();
        procedure redrawFrame();
        procedure startAnimation();
        procedure stopAnimation();
    end;

// Implementations

procedure setVector(var vect: TVector3; v0, v1, v2: Real);
begin
    vect[0] := v0;
    vect[1] := v1;
    vect[2] := v2;
end;

procedure TPolygon.setValues(i0, i1, i2: Word; pen_color: Color);
var
    indcs: TPolyIndecies;
begin
    indcs[0] := i0;
    indcs[1] := i1;
    indcs[2] := i2;
    self.indecies := indcs;
    self.penColor := pen_color;
end;

procedure mulVectXMatr(vector: TVector3; matrix: TMatrix3x3; 
    var res_vector: TVector3);
var
    v0, v1, v2: Real;
begin
    v0 := vector[0];
    v1 := vector[1];
    v2 := vector[2];
    res_vector[0] := v0 * matrix[0, 0] + v1 * matrix[1, 0] + v2 * matrix[2, 0];
    res_vector[1] := v0 * matrix[0, 1] + v1 * matrix[1, 1] + v2 * matrix[2, 1];
    res_vector[2] := v0 * matrix[0, 2] + v1 * matrix[1, 2] + v2 * matrix[2, 2];
end;

// Optimization required
procedure mulMatrXMatr(var matrix1: TMatrix3x3; matrix2: TMatrix3x3); // Bug
var
    r, c, e: Byte;
    mx     : TMatrix3x3;
begin
    for r := 0 to 2 do
    for c := 0 to 2 do
    begin
        mx[r, c] := 0;
        for e := 0 to 2 do
            mx[r, c] := mx[r, c] + matrix1[r, e] * matrix2[e, c];
    end; // c
    matrix1 := mx;
end;

procedure getRotateMatrix(var transf_mx: TMatrix3x3; angle: Real);
begin
    transf_mx[0, 0] := cos(angle);
    transf_mx[1, 0] := sin(angle);
    transf_mx[2, 0] := 0;
    transf_mx[0, 1] := 0 - transf_mx[1, 0];
    transf_mx[1, 1] := transf_mx[0, 0];
    transf_mx[2, 1] := 0;
    transf_mx[0, 2] := 0;
    transf_mx[1, 2] := 0;
    transf_mx[2, 2] := 1;
end;

procedure getScaleMatrix(var transf_mx: TMatrix3x3; scale: Real);
begin
    transf_mx[0, 0] := scale;
    transf_mx[1, 0] := 0;
    transf_mx[2, 0] := 0;
    transf_mx[0, 1] := 0;
    transf_mx[1, 1] := scale;
    transf_mx[2, 1] := 0;
    transf_mx[0, 2] := 0;
    transf_mx[1, 2] := 0;
    transf_mx[2, 2] := 1;
end;

procedure getDisplacementMatrix(var transf_mx: TMatrix3x3; disp: TPoint);
begin
    transf_mx[0, 0] := 1;
    transf_mx[1, 0] := 0;
    transf_mx[2, 0] := disp.x;
    transf_mx[0, 1] := 0;
    transf_mx[1, 1] := 1;
    transf_mx[2, 1] := disp.y;
    transf_mx[0, 2] := 0;
    transf_mx[1, 2] := 0;
    transf_mx[2, 2] := 1;
end;

procedure addDisplacementToMatrix(var transf_mx: TMatrix3x3; disp: TPoint);
begin
    transf_mx[2, 0] := disp.x;
    transf_mx[2, 1] := disp.y;
end;

procedure getIdentityMatrix(var transf_mx: TMatrix3x3);
begin
    transf_mx[0, 0] := 1;
    transf_mx[1, 0] := 0;
    transf_mx[2, 0] := 0;
    transf_mx[0, 1] := 0;
    transf_mx[1, 1] := 1;
    transf_mx[2, 1] := 0;
    transf_mx[0, 2] := 0;
    transf_mx[1, 2] := 0;
    transf_mx[2, 2] := 1;
end;

procedure resetAngle(var angle: Real);
begin
    if (angle > c2Pi) then angle -= c2Pi;
    if (angle < (-c2Pi)) then angle += c2Pi;
end;

procedure resetAngle(var angle, eps: Real);
begin
    if (angle > c2Pi) then angle -= c2Pi;
    if (angle < c2Pi) then angle += c2Pi;
end;

function angleToTangent(objectAngle, orbitAngle: Real): Real;
var
    res: Real;
begin
    res := orbitAngle - objectAngle + cPi;
    resetAngle(res);
    angleToTangent := res;
end;

procedure TRender.refresh();
begin
    self.numberOfObjects     := 0;
    self.lastVertexIndex     := 0;
    self.lastPolygonIndex    := 0;
    self.scene.numOfIndices  := 0;
    self.scene.numOfVertices := 0;
end;

procedure TRender.addObject(obj: TObject);
var
    currObjVertInd : Word;
    currObjPolyInd : Word;
    currScnVertInd : Word;
    currScnPolyInd : Word;
    cv             : TVector3; // Current vertex
    composeMatrix  : TMatrix3x3;
begin
    composeMatrix := obj.objectMatrix;
    mulMatrXMatr(composeMatrix, self.scene.worldMatrix);//Optimization required!
    mulMatrXMatr(composeMatrix, self.viewMatrix);       //Optimization required!
    currScnVertInd := self.lastVertexIndex;
    currScnPolyInd := self.lastPolygonIndex;
    for currObjPolyInd := 0 to obj.numOfIndices do
    begin
        self.scene.indices[currScnPolyInd]:= obj.indices[currObjPolyInd];
        inc(currScnPolyInd);
    end;
    for currObjVertInd := 0 to obj.numOfVertices do
    begin
        cv := obj.vertices[currObjVertInd];
        mulVectXMatr(obj.vertices[currObjVertInd], composeMatrix, 
            self.scene.vertices[currScnVertInd]);
        inc(currScnVertInd);
    end;
    self.lastVertexIndex  := currScnVertInd;
    self.lastPolygonIndex := currScnPolyInd;
    self.scene.numOfIndices  := currScnPolyInd - 1;
    self.scene.numOfVertices := currScnVertInd - 1;
end;

procedure TRender.renderFrame();
var
    cpind: Word;     // Current polygon index
    cvind: Word;     // Current vertex index
    cpoly: TPolygon; // Current polygon
    cpoints: array [0..2] of Point;
begin
    for cpind := 0 to self.scene.numOfIndices do
    begin
        cpoly := self.scene.indices[cpind];
        cvind := cpoly.indecies[0];
        cpoints[0].x := round(self.scene.vertices[cvind][0]);
        cpoints[0].y := round(self.scene.vertices[cvind][1]);
        cvind := cpoly.indecies[1];
        cpoints[1].x := round(self.scene.vertices[cvind][0]);
        cpoints[1].y := round(self.scene.vertices[cvind][1]);
        cvind := cpoly.indecies[2];
        cpoints[2].x := round(self.scene.vertices[cvind][0]);
        cpoints[2].y := round(self.scene.vertices[cvind][1]);
        setPenColor(cpoly.penColor);
        setBrushColor(cpoly.penColor);
        polygon(cpoints[0], cpoints[1], cpoints[2]); // Draw one polygon
    end;
end;

constructor TLabAnimation.create(fps: Byte; start_orbit, 
            target_orbit, wait_time, cranking_time, launch_time,
            max_launch_angle: Real);
begin
    self.setOrbitParams(start_orbit, target_orbit);
    self.rocket.mode           := TFlightMode.fmWaiting;
    self.rocket.waitingTime    := wait_time;
    self.rocket.crankingTime   := cranking_time;
    self.rocket.launchTime     := launch_time;
    self.rocket.currentTime    := 0;
    self.rocket.maxLaunchAngle := max_launch_angle;
    //self.createObjects();
    setWindowPos(400,0);
    //getIdentityMatrix(self.render.scene.worldMatrix);
    self.setFPS(fps);
    self.timerStart := false;
    //self.animTimer := new Timer(self.frameTime, self.redrawFrame);
end;

procedure TLabAnimation.setOrbitParams(start_orbit, target_orbit: Real);
begin
    self.rocket.startOrbitAltitude  := start_orbit;
    self.rocket.targetOrbitAltitude := target_orbit;
end;

procedure TLabAnimation.createObjects();
begin
    self.rocket.createAll();
end;

procedure TLabAnimation.setFPS(fps: Byte);
begin
    self.frameTime := 1.0 / Real(fps);
end;

procedure TLabAnimation.addObject(world_transform: TTransformations; 
    var obj: TObject);
var
    mx: TMatrix3x3;
begin
    getScaleMatrix(self.render.scene.worldMatrix, world_transform.scale);
    getRotateMatrix(mx, world_transform.angle);
    mulMatrXMatr(self.render.scene.worldMatrix, mx);
    addDisplacementToMatrix(self.render.scene.worldMatrix, 
        world_transform.displacement);
    self.render.addObject(obj);
end;

procedure TLabAnimation.updateScene(delta_time: Real);
var
    worldDisp  : TPoint;
begin
    self.rocket.changeState(delta_time);
    self.worldWidth  := 830;
    self.worldHeight := 830;
    worldDisp.x := windowWidth() /2;
    worldDisp.y := windowHeight()/2;
    if (windowHeight() < windowWidth()) 
    then begin
        self.scaleView := windowHeight()/self.worldHeight;
    end
    else begin
        self.scaleView := windowWidth()/self.worldWidth;
    end;
    getScaleMatrix(self.render.viewMatrix, self.scaleView);
    addDisplacementToMatrix(self.render.viewMatrix, worldDisp);
    self.addObject(self.rocket.transform, self.rocket.rocket);
end;

procedure TLabAnimation.drawBackGround();
const
    cTextSize    : Byte = 16;
    cTextInterval: Byte = 25;
var
    scaleTextSize: Byte;
    stringValue  : String;
    sec          : Byte;
    msec         : Byte;
begin
    clearWindow(clBlack);
    setPenColor(Color.LightBlue);
    setBrushColor(clBlack);
    scaleTextSize := Round(16 * self.scaleView);
    setFontSize(scaleTextSize);
    setFontColor(System.Drawing.Color.Aqua);
    textOut(round(5 * self.scaleView),round(
        5 * self.scaleView),                     'Time elapsed');
    textOut(round(5 * self.scaleView),round(
        (5 + cTextInterval) * self.scaleView),   'Flight mode');
    textOut(round(5 * self.scaleView),round(
        (5 + 2*cTextInterval) * self.scaleView), 'Orbit altitude');
    textOut(round(5 * self.scaleView),round(
        (5 + 3*cTextInterval) * self.scaleView), 'Orbit angle');
    textOut(round(5 * self.scaleView),round(
        (5 + 4*cTextInterval) * self.scaleView), 'Rocket angle');
    textOut(round(5 * self.scaleView),round(
        (5 + 5*cTextInterval) * self.scaleView), 'Rocket engine'); 
    str(self.rocket.currentTime:5:2, stringValue);
    textOut(round(150 * self.scaleView),round(
        (5) * self.scaleView),': ' + stringValue);
    str(self.rocket.orbitAltitude:5:2, stringValue);
    textOut(round(150 * self.scaleView),round(
        (5 + 2*cTextInterval) * self.scaleView),': ' + stringValue);
    str((self.rocket.orbitAngle * (-cRadToDeg)):5:2, stringValue);
    textOut(round(150 * self.scaleView),round(
        (5 + 3*cTextInterval) * self.scaleView),': ' + stringValue);
    str(((self.rocket.angle) * (-cRadToDeg)):5:2, stringValue);
    textOut(round(150 * self.scaleView),round(
        (5 + 4*cTextInterval) * self.scaleView), ': ' + stringValue);
    if (self.rocket.waitingTime > self.rocket.currentTime)
    then begin
        sec  := trunc(self.rocket.waitingTime - self.rocket.currentTime);
        msec := round(10*((self.rocket.waitingTime - self.rocket.currentTime) 
            - sec));
        if (msec = 10) 
            then msec := 0;
    end
    else begin
        sec  := 0;
        msec := 0;
    end;
    str(msec:2, stringValue);
    stringValue := intToStr(sec) + ':' + stringValue;
    case (self.rocket.mode) of
        TFlightMode.fmWaiting:
        begin
            textOut(round(150 * self.scaleView),round(
                (5 + cTextInterval) * self.scaleView),': Waiting');
            textOut(round(150 * self.scaleView),round(
                (5 + 5*cTextInterval) * self.scaleView),': Off');
            setFontSize(Round(30 * self.scaleView));
            textOut(((windowWidth() div 2) - round(40*self.scaleView)),
                round(10 * self.scaleView) ,stringValue);
        end;
        TFlightMode.fmCranking:
        begin
            textOut(round(150 * self.scaleView),round(
                (5 + cTextInterval) * self.scaleView),': Cranking');
            textOut(round(150 * self.scaleView),round(
                (5 + 5*cTextInterval) * self.scaleView),': On');
        end;
        TFlightMode.fmLaunch:
        begin
            textOut(round(150 * self.scaleView),round(
                (5 + cTextInterval) * self.scaleView),': Launch');
            textOut(round(150 * self.scaleView),round(
                (5 + 5*cTextInterval) * self.scaleView),': On');
        end;
        TFlightMode.fmOrbit:
        begin
            textOut(round(150 * self.scaleView),round(
                (5 + cTextInterval) * self.scaleView),': Orbit');
            textOut(round(150 * self.scaleView),round(
                (5 + 5*cTextInterval) * self.scaleView),': Off');
        end;
    end;
    setBrushColor(clGreen);
    circle(windowWidth() div 2, windowHeight() div 2, 
        Round(170 * self.scaleView));
end;

procedure TLabAnimation.redrawFrame();
var
    msDelay: Integer;
begin
    while (self.timerStart) do
    begin
        msDelay := Round(self.frameTime);
        sleep(msDelay);
        self.render.refresh();
        self.updateScene(self.frameTime);
        LockDrawing();
        self.drawBackGround();
        self.render.renderFrame();
        Redraw();
    end;
end;

procedure TLabAnimation.startAnimation();
begin
    //self.animTimer.Start();
    self.createObjects();
    getIdentityMatrix(self.render.scene.worldMatrix);
    self.timerStart := true;
    self.redrawFrame();
end;

procedure TLabAnimation.stopAnimation();
begin
    //self.animTimer.Stop();
    self.timerStart := false;
end;

procedure TRocket.createBody();
begin
    // Rocket height := 90
    self.rocket.vertices[0][0] := 10;
    setVector(self.rocket.vertices[0],  10, 30, 1);
    setVector(self.rocket.vertices[1],  30, 30, 1);
    setVector(self.rocket.vertices[2],  20, 0,  1);
    setVector(self.rocket.vertices[3],  10, 80, 1);
    setVector(self.rocket.vertices[4],  30, 80, 1);
    setVector(self.rocket.vertices[5],  10, 60, 1);
    setVector(self.rocket.vertices[6],   0, 70, 1);
    setVector(self.rocket.vertices[7],   0, 90, 1);
    setVector(self.rocket.vertices[8],  30, 60, 1);
    setVector(self.rocket.vertices[9],  40, 70, 1);
    setVector(self.rocket.vertices[10], 40, 90, 1);
    self.rocket.numOfVertices := 10; // Really - 11
    // Fairing
    self.rocket.indices[0].setValues(0, 1, 2, clBlue);
    // Tank
    self.rocket.indices[1].setValues(0, 1, 3, Color.Silver);
    self.rocket.indices[2].setValues(1, 3, 4, Color.Silver);
    // Right stabilizer
    self.rocket.indices[3].setValues(5, 6, 7, clBlue);
    self.rocket.indices[4].setValues(7, 3, 5, clBlue);
    // Left stabilizer
    self.rocket.indices[5].setValues(8, 9, 10, clBlue);
    self.rocket.indices[6].setValues(10, 4, 8, clBlue);
    self.rocket.numOfIndices := 6; // Really - 7
end;

procedure TRocket.createFire();
begin
    setVector(self.rocket.vertices[11],  13, 85, 1);
    setVector(self.rocket.vertices[12],  27, 85, 1);
    setVector(self.rocket.vertices[13],  20, 98, 1);
    
    setVector(self.rocket.vertices[14],  15, 85, 1);
    setVector(self.rocket.vertices[15],  25, 85, 1);
    setVector(self.rocket.vertices[16],  20, 92, 1);
    
    self.rocket.numOfVertices := 16; // Really - 17
    
    self.rocket.indices[7].setValues(11, 12, 13, clYellow);
    self.rocket.indices[8].setValues(14, 15, 16, clRed);
    
    self.rocket.numOfIndices := 8; // Really - 9
end;

procedure TRocket.changeFire();
var
    redFireSize   : Real;
    yellowFireSize: Real;
begin
    redFireSize := 90 + random(10);
    yellowFireSize := redFireSize + 3 + random(12);
    setVector(self.rocket.vertices[16],  
        22 - random(4), redFireSize   , 1); // Red fire
    setVector(self.rocket.vertices[13],  
        21 - random(2), yellowFireSize, 1); // Yellow fire
end;

procedure TRocket.createAll();
var
    tLaunch2: Real;
    objDisp  : TPoint;
begin
    self.createBody();
    self.createFire();
    
    tLaunch2 := self.launchTime * self.launchTime;

    self.orbitAltitude             := self.startOrbitAltitude; 
    self.orbitAltitudeSpeed        := 0.0;
    // s(t) = s0 + v0*t + (a*t^2)/2; {s - altitude}
    self.orbitAltitudeAcceleration := 2*(self.targetOrbitAltitude - 
        self.startOrbitAltitude)/(tLaunch2);
    // 
    self.orbitAngle                := 0.0 - cPi;
    self.orbitAngleVelocity        := 0.0;
    // r(t) = r0 + w0*t + (e*t^2)/2; {r - angle}
    self.orbitAngularAcceleration  := 0 - (2 * self.maxLaunchAngle)/(tLaunch2);
    
    self.angle                     := 0.0 - c2Pi;
    self.angularVelocity           := 0.0;
    self.angularAcceleration       := self.orbitAngularAcceleration - 
        cPi/(tLaunch2);
    
    self.launchTime := 0;
    objDisp.x := -20 * 1.5;
    objDisp.y := -45 * 1.5;
    getDisplacementMatrix(self.rocket.objectMatrix, objDisp);
    
    if (self.mode = TFlightMode.fmWaiting) 
        then self.rocket.numOfIndices := 6; // No fire
    self.launchTime := 0.0;
end;

procedure TRocket.changeState(delta_time: Real); 
begin
    self.transform.angle          := self.angle;
    self.transform.scale          := 1.5;
    self.transform.displacement.x := self.orbitAltitude * sin( self.orbitAngle);
    self.transform.displacement.y := self.orbitAltitude * cos( self.orbitAngle);
    case (self.mode) of
        TFlightMode.fmWaiting:
        begin
            if (self.currentTime > self.waitingTime) 
            then begin 
                self.mode := TFlightMode.fmCranking;
                self.rocket.numOfIndices := 8; // Fire
            end;
        end;
        TFlightMode.fmCranking:
        begin
            if (self.currentTime > (self.waitingTime + self.crankingTime)) 
            then begin
               self.mode := TFlightMode.fmLaunch;
            end;
        end;
        TFlightMode.fmLaunch:
        begin
            if (self.orbitAltitude >= 350) then
            begin
                self.mode                      := TFlightMode.fmOrbit;
                self.rocket.numOfIndices := 6; // No fire
                self.orbitAngularAcceleration  := 0.0;
                self.orbitAltitudeAcceleration := 0.0;
                self.orbitAltitudeSpeed        := 0.0;
            end;
            self.angle              += self.angularVelocity           
                * delta_time;
            self.orbitAltitude      += self.orbitAltitudeSpeed        
                * delta_time;
            self.orbitAngleVelocity += self.orbitAngularAcceleration  
                * delta_time;
            self.angularVelocity    += self.angularAcceleration       
                * delta_time;
            self.orbitAltitudeSpeed += self.orbitAltitudeAcceleration 
                * delta_time;
        end;
        TFlightMode.fmOrbit:
        begin
            self.angle := self.orbitAngle + cPiDiv2;
        end;
    end;
    self.orbitAngle    += self.orbitAngleVelocity * delta_time;
    resetAngle(self.orbitAngle);
    resetAngle(self.angle);
    self.changeFire();
    self.currentTime += delta_time;
end;

// General program
var 
    labAnim   : TLabAnimation;
    //vec: TRocket;
begin
    Randomize;
    
    //MaximizeWindow;
    labAnim := new TLabAnimation(cDefaultFramesPerSecond, 200.0, 350.0, 3.1, 
        1.0, 5.0, cPiDiv2/1.5);
    labAnim.startAnimation();
    writeLn();
end.