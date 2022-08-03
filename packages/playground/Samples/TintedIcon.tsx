 import React from 'react';
 import {
   Animated,
   Image,
   ImageSourcePropType,
   ImageStyle,
   StyleProp,
 } from 'react-native';

 type Props = {
   size: number;
   icon: ImageSourcePropType;
   color?: string;
   style?: StyleProp<ImageStyle>;
   resizeMode?: 'center' | 'contain' | 'cover' | 'stretch' | 'repeat';
   testID?: string;
 };

 export function AnimatedTintedIcon(
   props: Omit<Props, 'style'> & {
     style?: Animated.WithAnimatedValue<Props['style']>;
   },
 ) {
   return (
     <Animated.Image
       source={props.icon}
       style={[
         {
           width: props.size,
           height: props.size,
           tintColor: props.color || undefined,
         },
         props.style,
       ]}
       resizeMode={props.resizeMode ?? undefined}
     />
   );
 }

 export default function TintedIcon(props: Props) {
   return (
     <Image
       source={props.icon}
       style={[
         {
           width: props.size,
           height: props.size,
           tintColor: props.color || undefined,
         },
         props.style,
       ]}
       resizeMode={props.resizeMode ?? undefined}
       testID={props.testID}
     />
   );
 }
