# Walkmate

Walkmate is a Pebble watch face for daily walking progress.

It shows the current date and time, today's step count, walking distance, a progress ring for your daily step goal, and optional temperature and battery gauges.

## Features

- Date display in `MMM/D` or `MMM/DD` format
- Time display using the watch's 12-hour or 24-hour setting
- Today's step count from Pebble Health
- Today's walked distance in kilometers
- Circular progress ring based on a configurable daily step goal
- Overflow indicator when the step goal is exceeded
- Configurable progress ring color
- Optional auxiliary gauges:
  - Weather temperature gauge using current, daily high, and daily low temperatures
  - Battery gauge using the watch battery level
- Phone-side configuration page
- Persistent settings on both the watch and phone side

## Weather

Walkmate requests weather data from the phone through PebbleKit JS.

- The phone obtains the current location with `navigator.geolocation`.
- Weather data is fetched from the Open-Meteo forecast API.
- The watch stores the latest received temperature values and redraws the temperature gauge from them.
- The refresh interval is configurable.

Weather and battery gauges can be hidden from the configuration page.

## Configuration

Open the watch face settings from the Pebble mobile app.

Available settings:

| Setting | Default | Range / Options |
| --- | ---: | --- |
| Daily step goal | `10000` | `1000` to `99999` steps |
| Ring color | White | White, Blue, Green, Yellow, Orange, Red, Purple, Pink |
| Ring color on monochrome watches | White | White, Grey |
| Weather update interval | `30` minutes | `5` to `180` minutes |
| Show temperature and battery gauges | On | On / Off |
| Temperature gauge range | `-10` to `40` C | `-50` to `60` C, minimum below maximum |

## Project Structure

- `src/c/Walkmate.c` - Pebble watch face implementation
- `src/pkjs/index.js` - PebbleKit JS configuration page and weather bridge
- `package.json` - Pebble project metadata, platforms, message keys, capabilities, and resources
- `resources/images/Walkmate.png` - App icon
- `resources/fonts/` - Custom Outfit fonts used by the watch face
- `wscript` - Pebble SDK build script
- `LICENSE` - BSD 3-Clause License

## Requirements

- Pebble SDK 3
- A Pebble platform supported by the project:
  - Aplite
  - Basalt
  - Chalk
  - Diorite
  - Emery
  - Flint
  - Gabbro
- Pebble Health support for step count and walked distance
- Phone location permission for weather updates

## Build

Run from the project root:

```sh
pebble build
```

## Install

Install through a connected phone:

```sh
pebble install --phone <device-ip-address>
```

Or install directly to a connected watch:

```sh
pebble install
```

## License

This project is licensed under the BSD 3-Clause License. See [LICENSE](LICENSE).

---

## 日本語

Walkmate は、日々のウォーキング進捗を表示する Pebble 向けウォッチフェイスです。

現在の日付と時刻、今日の歩数、歩行距離、目標歩数に対する進捗リング、任意表示の気温ゲージとバッテリーゲージを表示します。

## 機能

- `MMM/D` または `MMM/DD` 形式の日付表示
- ウォッチ本体の設定に合わせた 12 時間 / 24 時間表示
- Pebble Health から取得した今日の歩数
- 今日の歩行距離を km 表示
- 設定可能な目標歩数に基づく円形進捗リング
- 目標歩数を超えた場合のオーバーフロー表示
- 進捗リング色の設定
- 任意表示の補助ゲージ:
  - 現在気温、最高気温、最低気温を使った気温ゲージ
  - ウォッチのバッテリー残量ゲージ
- スマートフォン側の設定画面
- ウォッチ側とスマートフォン側の設定永続化

## 天気

Walkmate は PebbleKit JS 経由でスマートフォンに天気データを要求します。

- スマートフォン側で `navigator.geolocation` を使って現在地を取得します。
- Open-Meteo forecast API から天気データを取得します。
- ウォッチ側は受け取った気温値を保存し、その値を使って気温ゲージを再描画します。
- 更新間隔は設定画面から変更できます。

気温ゲージとバッテリーゲージは、設定画面から非表示にできます。

## 設定

Pebble モバイルアプリからウォッチフェイスの設定を開きます。

設定項目:

| 項目 | デフォルト | 範囲 / 選択肢 |
| --- | ---: | --- |
| 1 日の目標歩数 | `10000` | `1000` から `99999` 歩 |
| リング色 | White | White, Blue, Green, Yellow, Orange, Red, Purple, Pink |
| モノクロウォッチでのリング色 | White | White, Grey |
| 天気更新間隔 | `30` 分 | `5` から `180` 分 |
| 気温・バッテリーゲージ表示 | オン | オン / オフ |
| 気温ゲージ範囲 | `-10` から `40` C | `-50` から `60` C、下限が上限未満 |

## プロジェクト構成

- `src/c/Walkmate.c` - Pebble ウォッチフェイス本体
- `src/pkjs/index.js` - PebbleKit JS の設定画面と天気連携
- `package.json` - Pebble プロジェクト定義、対応プラットフォーム、メッセージキー、権限、リソース
- `resources/images/Walkmate.png` - アプリアイコン
- `resources/fonts/` - ウォッチフェイスで使う Outfit カスタムフォント
- `wscript` - Pebble SDK ビルドスクリプト
- `LICENSE` - BSD 3-Clause License

## 必要なもの

- Pebble SDK 3
- このプロジェクトが対応する Pebble プラットフォーム:
  - Aplite
  - Basalt
  - Chalk
  - Diorite
  - Emery
  - Flint
  - Gabbro
- 歩数と歩行距離を取得するための Pebble Health
- 天気更新に使うスマートフォン側の位置情報権限

## ビルド

プロジェクトルートで実行します。

```sh
pebble build
```

## インストール

接続済みのスマートフォン経由でインストールする場合:

```sh
pebble install --phone <デバイスのIPアドレス>
```

接続済みのウォッチへ直接インストールする場合:

```sh
pebble install
```

## ライセンス

このプロジェクトは BSD 3-Clause License のもとで公開されています。詳細は [LICENSE](LICENSE) を参照してください。
