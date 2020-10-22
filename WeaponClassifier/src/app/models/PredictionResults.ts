export interface Prediction {
    TagId: string;
    Tag: string;
    Probability: number;
}

export interface Result {
    Id: string;
    Predictions: Prediction[];
}
