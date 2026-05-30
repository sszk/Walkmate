# Walkmate

Walkmate is a Pebble watch face for daily walking progress.

It shows the current date and time, today's step count, walked distance, a progress ring for your daily step goal, and optional temperature and battery gauges.

## Features

- Date display in `MMM/D` or `MMM/DD` format
- Time display using the watch's 12-hour or 24-hour setting
- Today's step count from Pebble Health, capped at `99999` in the center display
- Today's walked distance from Pebble Health, shown in kilometers with one decimal place
- Circular progress ring based on a configurable daily step goal
- Overflow indicator when the step goal is exceeded
- Configurable progress ring color
- Layout and font sizes selected automatically for small, medium, and large Pebble displays
- Optional auxiliary gauges:
  - Upper weather temperature gauge using current, daily high, and daily low temperatures
  - Lower battery gauge using the watch battery level
  - On color watches, the temperature gauge is split by temperature bands and the battery gauge is colored by remaining charge
  - On monochrome watches, auxiliary gauges use the original white/dark gray rendering, including charging-state highlighting for the battery gauge
- Tap-to-refresh weather with a temporary center temperature display (`current` and `high/low`)
- Phone-side configuration page
- Persistent settings on both the watch and phone side

## Weather

Walkmate requests weather data from the phone through PebbleKit JS.

- The phone obtains the current location with `navigator.geolocation`.
- Weather data is fetched from the Open-Meteo forecast API using current `temperature_2m` and daily `temperature_2m_max` / `temperature_2m_min`.
- Temperatures are rounded to whole degrees Celsius before they are sent to the watch.
- The watch stores the latest received temperature values and redraws the temperature gauge from them.
- On color watches, the temperature gauge is split into colored bands: dark blue below `-15` C, blue from `-15` C to `-5` C, cyan from `-5` C to `5` C, green from `5` C to `20` C, yellow from `20` C to `30` C, orange from `30` C to `35` C, red from `35` C to `40` C, and purple above `40` C. The current temperature marker remains white.
- Weather is requested when the watch face loads, when the configured refresh interval has elapsed, and when the watch is tapped.
- Tapping the watch requests fresh weather data and temporarily replaces the center step display with current temperature and `high/low` temperature. If no temperature is available, the preview shows `--°C` and `--/--°C`.
- Weather requests on the watch side time out after 30 seconds so the tap preview can still finish.

Weather and battery gauges can be hidden from the configuration page.

On color watches, the battery gauge is green at `50%` or above, yellow from `20%` to `49%`, and red below `20%`.

## Configuration

Open the watch face settings from the Pebble mobile app.

Available settings:

| Setting | Default | Range / Options |
| --- | ---: | --- |
| Daily step goal | `10000` | `1000` to `99999` steps |
| Ring color | White | White, Blue, Green, Yellow, Orange, Red, Purple, Pink |
| Ring color on monochrome watches | White | White, Grey |
| Weather update interval | `30` minutes | `5` to `180` minutes |
| Temperature display duration | `5` seconds | `0` to `10` seconds; `0` disables tap temperature display |
| Show temperature and battery gauges | On | On / Off |
| Temperature gauge range | Automatic | Manual `-50` to `60` C, or automatic from today's high/low with padding |

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
- Pebble Health support for step count and walked distance. If health data is unavailable, step and distance values are treated as `0`.
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
- Pebble Health から取得した今日の歩数。中央表示では `99999` が上限です
- Pebble Health から取得した今日の歩行距離を、小数 1 桁の km で表示
- 設定可能な目標歩数に基づく円形進捗リング
- 目標歩数を超えた場合のオーバーフロー表示
- 進捗リング色の設定
- Pebble の画面サイズに応じた小 / 中 / 大レイアウトとフォントサイズの自動選択
- 任意表示の補助ゲージ:
  - 現在気温、最高気温、最低気温を使った上側の気温ゲージ
  - ウォッチのバッテリー残量を使った下側のバッテリー残量ゲージ
  - カラーウォッチでは、気温ゲージを温度帯ごとに色分けし、バッテリーゲージを残量ごとに色分けします
  - モノクロウォッチでは、補助ゲージは従来の白 / 濃灰表示を使い、バッテリーゲージでは充電状態も反映します
- タップによる天気更新と、一時的な中央気温表示（現在気温と最高/最低気温）
- スマートフォン側の設定画面
- ウォッチ側とスマートフォン側の設定永続化

## 天気

Walkmate は PebbleKit JS 経由でスマートフォンに天気データを要求します。

- スマートフォン側で `navigator.geolocation` を使って現在地を取得します。
- Open-Meteo forecast API から、現在の `temperature_2m` と日別の `temperature_2m_max` / `temperature_2m_min` を取得します。
- 気温は整数の摂氏に丸めてからウォッチへ送信します。
- ウォッチ側は受け取った気温値を保存し、その値を使って気温ゲージを再描画します。
- カラーウォッチでは、気温の範囲を温度帯ごとに色分けして描画します。`-15` C 未満は濃紺、`-15` C から `-5` C は青、`-5` C から `5` C はシアン、`5` C から `20` C は緑、`20` C から `30` C は黄色、`30` C から `35` C はオレンジ、`35` C から `40` C は赤、それ以上は紫です。現在気温のマーカーは白のままです。
- ウォッチフェイス読み込み時、設定した更新間隔の経過時、ウォッチのタップ時に天気データを要求します。
- ウォッチをタップすると天気データを更新し、中央の歩数表示を一時的に現在気温と最高/最低気温の表示へ切り替えます。気温が未取得の場合は `--°C` と `--/--°C` を表示します。
- ウォッチ側の天気リクエストは 30 秒でタイムアウトし、タップ時プレビューの待機を終了します。

気温ゲージとバッテリーゲージは、設定画面から非表示にできます。

カラーウォッチでは、バッテリーゲージは `50%` 以上で緑、`20%` から `49%` で黄色、`20%` 未満で赤になります。

## 設定

Pebble モバイルアプリからウォッチフェイスの設定を開きます。

設定項目:

| 項目 | デフォルト | 範囲 / 選択肢 |
| --- | ---: | --- |
| 1 日の目標歩数 | `10000` | `1000` から `99999` 歩 |
| リング色 | White | White, Blue, Green, Yellow, Orange, Red, Purple, Pink |
| モノクロウォッチでのリング色 | White | White, Grey |
| 天気更新間隔 | `30` 分 | `5` から `180` 分 |
| 気温表示時間 | `5` 秒 | `0` から `10` 秒。`0` でタップ時の気温表示を無効化 |
| 気温・バッテリーゲージ表示 | オン | オン / オフ |
| 気温ゲージ範囲 | 自動 | 手動では `-50` から `60` C、または今日の最高/最低気温から余白付きで自動設定 |

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
- 歩数と歩行距離を取得するための Pebble Health。利用できない場合、歩数と距離は `0` として扱われます
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
