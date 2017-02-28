import QtQuick 2.0

Item {
    id: root

    property alias angle: wheel.angle;
    property alias wheelWidth: wheel.wheelWidth;
    property real wheelRadius: (Math.min(parent.width, parent.height) - 10)/2
    property alias borderColor: wheel.borderColor;

    signal editingFinished;

    Item{
        id: wheel

        property real wheelWidth: 30

        property real angle: 0;
        property color borderColor: "#555555"

        onAngleChanged: cursor.color = Qt.hsva(angle, 1,0.9,1);

        anchors.centerIn: parent
        width: 2*root.wheelRadius
        height: width

        MouseArea{
            property bool acceptPress: false;
            anchors.fill: parent
            onPositionChanged:  applyMousePos(mouse.x, mouse.y, false)
            onPressed:          applyMousePos(mouse.x, mouse.y, true);
            onReleased: acceptPress = false;

            function applyMousePos( posX, posY, checkPos){
                posX = posX-(height/2);
                posY = posY-( width/2);
                var radius = Math.sqrt(posX*posX + posY*posY)/wheel.width*2;
                if(checkPos)
                    acceptPress = radius>wheelGradient.innerRadius && radius<wheelGradient.outerRadius;

                if(acceptPress){
                    var radAngle = - Math.PI/2 + Math.atan2(posY, posX);
                    wheel.angle = radAngle*0.5/Math.PI + 0.5;
                    wheel.angle = -wheel.angle ;
                    wheel.angle = wheel.angle + (wheel.angle<0?1:0);
                    root.editingFinished();
                }
            }
        }

        Rectangle{
            id: wheelGradient
            anchors.fill: parent

            property real innerRadius: 1- 2*(wheel.wheelWidth / wheel.width)
            property real outerRadius: 1
                        layer.enabled: true
            layer.effect:ShaderEffect {
                property real innerRadius: wheelGradient.innerRadius
                property real outerRadius: wheelGradient.outerRadius

                fragmentShader: "
        #define PI 3.14159265358979323844
        varying highp vec2 qt_TexCoord0;
        uniform float innerRadius;
        uniform float outerRadius;

        vec3 hsv2rgb(vec3 c)
        {
            vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        void main() {
            vec2 coord = 2.0*qt_TexCoord0 - vec2(1.0,1.0);
            float radius = length(coord);
            float angle  = atan(coord.x, coord.y);
            angle = angle / (2.0 * PI) + 0.5;
            float alpha = (radius>innerRadius && radius<outerRadius)?1.0:0.0;
            gl_FragColor = vec4( hsv2rgb(vec3( angle, 0.3, 1)), alpha);
        }
    "
            }
        }

        Rectangle{
            id: cursor
            width: 6
            radius: 6
            height: wheel.wheelWidth - 4
            x: (parent.width)/2
            y: 3
            color: Qt.hsva(0, 1,0.9,1);

            transform: [
                Rotation{angle: -360*wheel.angle; origin.x: 0; origin.y: (wheel.height)/2 - cursor.y;}
            ]
        }

        Rectangle{
            id: outerBorder
            radius: width/2
            anchors.fill: parent

            border.width: 2
            border.color: wheel.borderColor
            color: "#00ffffff"
        }
        Rectangle{
            id: innerBorder
            radius: width/2
            anchors.fill: parent
            anchors.margins: wheel.wheelWidth
            border.width: 2
            border.color: wheel.borderColor
            color: "#00ffffff"
        }
    }
}
