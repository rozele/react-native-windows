/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */
import React, {useState} from 'react';
import StarRating from './StarRating';
import {AppRegistry, Button, Text} from 'react-native';

export default function FeedbackStars() {
  const [rating, setRating] = useState(0);
  const [useNativeDriver, setUseNativeDriver] = useState(false);
  return (
    <>
      <StarRating rating={rating} onRatingChange={setRating} useNativeDriver={useNativeDriver} />
      <Text style={{marginTop: 12}}>
        Number of selected stars: {rating}
      </Text>
      <Button title={useNativeDriver ? 'Use JS Driver' : 'Use Native Driver'} onPress={() => setUseNativeDriver(!useNativeDriver)} />
    </>
  );
}

AppRegistry.registerComponent('Bootstrap', () => FeedbackStars);
